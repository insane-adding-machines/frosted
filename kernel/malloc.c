/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the Free Software Foundation.
 *      
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "malloc.h"
#include "scheduler.h"
#include "frosted.h"
#include "locks.h"

/*------------------*/
/* Defines          */
/*------------------*/
#define F_MALLOC_MAGIC    (0xDECEA5ED) 


#if defined __linux__ || defined _WIN32 /* test application */
#define dbg_malloc printf
#else
#define dbg_malloc(...) do{}while(0)
#endif

#define F_IN_USE 0x20
#define in_use(x) (((x->flags) & F_IN_USE) == F_IN_USE)
#define block_valid(b) ((b) && (b->magic == F_MALLOC_MAGIC))

static inline int MEMPOOL(int x)
{
    if (x == MEM_TCPIP)
        return 3;
    if (x == MEM_EXTRA)
        return 4;
    return x;
}

/*------------------*/
/* Structures       */
/*------------------*/
struct f_malloc_block {
    uint32_t magic;                 /* magic fingerprint */
    struct f_malloc_block * prev;   /* previous block */
    struct f_malloc_block * next;   /* next, or last block? */
    size_t size;                    /* malloc size excluding this block - next block is adjacent, if !last_block */
    uint32_t flags; 
    int pid;
};


/*------------------*/
/* Local variables  */
/*------------------*/
static struct f_malloc_block *malloc_entry[4] = {NULL, NULL, NULL, NULL};

/* Globals */
struct f_malloc_stats f_malloc_stats[4] = {};

/* Mlock is a special lock, so initialization is made static */
static int _m_listeners[16] = { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 };
static struct semaphore _mlock = { .value = 1, .listeners=16, .listener=_m_listeners};
static mutex_t *mlock = (mutex_t *)(&_mlock);

#define KMEM_SIZE   (CONFIG_KRAM_SIZE << 10)

/*------------------*/
/* Local functions  */
/*------------------*/

/* merges two blocks
 *  returns: pointer to the merged block
 */
static struct f_malloc_block * merge_blocks(struct f_malloc_block * first, struct f_malloc_block * second)
{
    struct f_malloc_block *temp;
    if (first > second) {
        temp = first;
        first = second;
        second = temp;
    }
    /* new size = sum of sizes + overhead */
    first->size = first->size + sizeof(struct f_malloc_block) + second->size;
    /* first block's next pointer should now point the second blocks next pointer */
    first->next = second->next;
    /* third block's prev pointer should now point to the first block, instead of the second */
    if (second->next)
        second->next->prev = first;
    return first;
}

/* split a block in two.
 * blk: block to be split
 * size: size of the first block
 * return: NULL on failure, otherwise pointer to the new free block (second block)
 * NOTE: the second block will be put in the free-blocks pool, with the remaining size
 */
static struct f_malloc_block * split_block(struct f_malloc_block * blk, size_t size)
{
    size_t free_size;
    struct f_malloc_block * free_blk;
  
    /* does it fit? */
    if ((blk->size - sizeof(struct f_malloc_block)) <= size)
        return NULL;
  
    /* shrink the block to requested size */
    free_size = blk->size - sizeof(struct f_malloc_block) - size;
    blk->size = size;
    /* create new block */
    free_blk = (struct f_malloc_block *)(((uint8_t *)blk) + sizeof(struct f_malloc_block) + blk->size);
    free_blk->prev = blk;
    free_blk->next = blk->next;
    blk->next = free_blk;
    free_blk->magic = F_MALLOC_MAGIC;
    free_blk->size = free_size;
    free_blk->flags = 0u;
    return free_blk;
}

static int block_fits(struct f_malloc_block *blk, size_t size, int flags)
{
    uint32_t baddr = (uint32_t)blk;
    uint32_t reqsize = size + sizeof(struct f_malloc_block);
    if (!blk)
        return 0;

    if (!block_valid(blk))
        return 0;

    if (in_use(blk))
        return 0;

    if (size > blk->size)
        return 0;
    
    return 1;
}

static struct f_malloc_block * f_find_best_fit(int flags, size_t size, struct f_malloc_block ** last)
{
    struct f_malloc_block *found = NULL, *blk = malloc_entry[MEMPOOL(flags)];

    /* See if we can find a free block that fits */
    while (blk) /* last entry will break the loop */
    {
        *last = blk; /* last travelled node */

        if (block_fits(blk, size, flags))
        {
            /* found a fit - is it better than a possible previously found block? */
            if ((!found) || (blk->size < found->size))
                found = blk;
        }
        if ((blk->next) && (!block_valid(blk->next)))
            blk->next = NULL;

        /* travel next block */
        blk = blk->next;
    }
    return found;
}

static unsigned char * heap_end_kernel;
static unsigned char * heap_stack;
static unsigned char * heap_end_user;
static unsigned char * heap_end_extra;

static unsigned char * heap_limit_user;
static unsigned char * heap_limit_kernel;

#ifdef CONFIG_TCPIP_MEMPOOL
    static char * heap_end_tcpip = CONFIG_TCPIP_MEMPOOL;
#else
    static char * heap_end_tcpip = NULL;
#endif

static void * f_sbrk(int flags, int incr)
{
    extern char   end;              /* Set by linker */
    extern char   _stack;           /* Set by linker */
    extern char   _user_heap_start; /* Set by linker */
    extern char   _user_heap_end;   /* Set by linker */
#if defined(CONFIG_SRAM_EXTRA) || defined(CONFIG_SDRAM)
    extern char   _extra_heap_start; /* Set by linker */
    extern char   _extra_heap_end;   /* Set by linker */
#endif
    const char  * heap_stack_high = &_stack;
    char        * prev_heap_end;

    /* Set initial heap addresses */
    if (heap_end_kernel == 0) {
        /* kernel memory */
        heap_end_kernel = &end;
        heap_limit_kernel = &end + KMEM_SIZE;

#ifdef CONFIG_SDRAM
        /* user memory  = external ram*/
        heap_end_user = &_extra_heap_start; /* Start at beginning of heap */
        heap_limit_user = &_extra_heap_end;
#else
        /* user memory */
        heap_end_user = &_user_heap_start; /* Start at beginning of heap */
        heap_limit_user = &_user_heap_end;
#endif

        /* task/stack memory */
        heap_stack = &_stack - 4096; 
    }

    if (flags & MEM_USER) {
        if (!heap_end_user)
            return (void *)(0 - 1);
        /* Do not over-commit */
        if ((heap_end_user + incr) > heap_limit_user)
            return (void *)(0 - 1);
        prev_heap_end = heap_end_user;
        heap_end_user += incr;
        memset(prev_heap_end, 0, incr);
#ifdef CONFIG_SRAM_EXTRA
    } else if (flags & MEM_EXTRA) {
        if (!heap_end_extra)
            return (void *)(0 - 1);
        /* Do not over-commit */
        if ((heap_end_extra + incr) > &_extra_heap_end)
            return (void *)(0 - 1);
        prev_heap_end = heap_end_extra;
        heap_end_extra += incr;
        memset(prev_heap_end, 0, incr);
#endif
    }else if (flags & MEM_TASK) {
        if ((heap_stack - incr) < heap_end_kernel)
            return (void *)(0 - 1);
        heap_stack -= incr;
        prev_heap_end = heap_stack;
    } else if (flags & MEM_TCPIP) {
        if (!heap_end_tcpip)
            return (void *)(0 - 1);
        prev_heap_end = heap_end_tcpip;
        heap_end_tcpip += incr;
    } else {
        if ((heap_end_kernel + incr) > heap_limit_kernel)
            return (void*)(0 - 1);
        prev_heap_end = heap_end_kernel;
        heap_end_kernel += incr;
    }
    return (void *) prev_heap_end;
}

static void f_compact(struct f_malloc_block *blk)
{
    if (!blk->prev)
        return;
    blk->prev->next = NULL;
    if (blk->flags & MEM_USER) {
        heap_end_user -= (blk->size + sizeof(struct f_malloc_block));
    } else if (blk->flags & MEM_TASK) {
        heap_stack += (blk->size + sizeof(struct f_malloc_block));
    } else if (blk->flags & MEM_TCPIP) {
        heap_end_tcpip -= (blk->size + sizeof(struct f_malloc_block));
    } else {
        heap_end_kernel -= (blk->size + sizeof(struct f_malloc_block));
    }
}

/*------------------*/
/* Public functions */
/*------------------*/
void * f_calloc(int flags, size_t num, size_t size)
{
    void * ptr = f_malloc(flags, num * size);
    if (ptr)
        memset(ptr, 0, num * size);
    return ptr;
}

void* f_realloc(int flags, void* ptr, size_t size)
{
    void * out = NULL;
    struct f_malloc_block * blk;

    /* size zero and valid ptr -> act as regular free() */
    if (!size && ptr)
        goto realloc_free;
    
    blk = (struct f_malloc_block *)(((uint8_t*)ptr) - sizeof(struct f_malloc_block));



    if (!ptr)
    {
        /* f ptr is not valid, act as regular malloc() */
        out = f_malloc(flags, size);
    }
    else if (block_valid(blk))
    {
        size_t new_size, copy_size;
        if ((blk->flags & F_IN_USE) == 0) {
            task_segfault((uint32_t)ptr, 0, MEMFAULT_ACCESS);
        }

        /* If requested size is the same as current, do nothing. */
        if (blk->size == size)
            return ptr;
        else if (size > blk->size)
        {
            /* Grow to requested size by copying the content */
            new_size = size;
            copy_size = blk->size;
        } else {
            /* Shrink  (Ignore for now) */
            return ptr;
        }
        out = f_malloc(flags, size);
        if (!out)  {
            return NULL;
        }
        memcpy(out, ptr, copy_size);
    }

realloc_free:
    if (ptr)
        f_free(ptr);
    return out;
}

void f_proc_heap_free(int pid)
{
    struct f_malloc_block *blk = malloc_entry[MEM_USER];
    while (blk)
    {
        if (!block_valid(blk))
            while(1); /* corrupt block! */
        if ((blk->pid == pid) && in_use(blk)) {
            f_free(blk);
        }
        blk = blk->next;
    }
}

uint32_t f_proc_heap_count(int pid)
{
    struct f_malloc_block *blk = malloc_entry[MEM_USER];
    uint32_t size = 0;
    while (blk)
    {
        if ((blk->pid == pid) && in_use(blk)) {
            size += blk->size;
        }
        blk = blk->next;
    }
    return size;
}

void * f_malloc(int flags, size_t size)
{
    struct f_malloc_block * blk = NULL, *last = NULL;
    void *ret = NULL;
    while((size % 4) != 0) {
        size++;
    } 

    if (scheduler_get_cur_pid() == 0) {
        if (mutex_trylock(mlock) < 0) {
            return NULL;
        }
    } else {
        mutex_lock(mlock);
    }

    /* update stats */
    f_malloc_stats[MEMPOOL(flags)].malloc_calls++;

    /* Travel the linked list for first fit */
    blk = f_find_best_fit(flags, size, &last);
    if (blk)
    {
        dbg_malloc("Found best fit!\n");
        /*
         * if ((flags & MEM_USER) && blk->next && ((uint8_t *)blk + 24 + blk->size != (uint8_t *)blk->next))
         *      while(1);;
         */
        /* first fit found, now split it if it's much bigger than needed */
        if (2 * (size + sizeof(struct f_malloc_block)) < blk->size)
        {
            dbg_malloc("Splitting blocks, since requested size [%d] << best fit block size [%d]!\n", size, blk->size);
            split_block(blk, size);
        }
    } else {
        /* No first fit found: ask for new memory */
        blk = (struct f_malloc_block *)f_sbrk(flags, size + sizeof(struct f_malloc_block));  // can OS give us more memory?
        if ((long)blk == -1) {
            mutex_unlock(mlock);
            return NULL;
        }

        /* first call -> set entrypoint */
        if (malloc_entry[MEMPOOL(flags)] == NULL) {
            malloc_entry[MEMPOOL(flags)] = blk;
            blk->prev = NULL;
        }
        blk->magic = F_MALLOC_MAGIC;
        blk->size = size;
        blk->next = NULL; /* Newly allocated memory is always last in the linked list */
        /* Link this entry to the previous last entry */
        if (last)
        {
            // assert(last->next == NULL);
            last->next = blk;
        }
        blk->prev = last;
    }

    /* destination found, fill in  meta-data */
    blk->flags = F_IN_USE | flags;
    if (flags & MEM_USER)
        blk->pid = scheduler_get_cur_pid();
    else
        blk->pid = 0;

    /* update stats */
    f_malloc_stats[MEMPOOL(flags)].objects_allocated++;
    f_malloc_stats[MEMPOOL(flags)].mem_allocated += ((uint32_t)blk->size + sizeof(struct f_malloc_block));

    ret = (void *)(((uint8_t *)blk) + sizeof(struct f_malloc_block)); // pointer to newly allocated mem
    mutex_unlock(mlock);
    return ret;
}

static void blk_rearrange(void *arg)
{
    struct f_malloc_block *blk = arg;


    /* Merge adjecent free blocks (consecutive blocks are always adjacent) */
    if ((blk->prev) && (!in_use(blk->prev)))
    {
        blk = merge_blocks(blk->prev, blk);
    }
    if ((blk->next) && (!in_use(blk->next)))
    {
        blk = merge_blocks(blk, blk->next);
    }
    if (!blk->next)
        f_compact(blk);
}


void f_free(void * ptr)
{
    struct f_malloc_block * blk;

    if (!ptr) {
        return;
    }

    blk = (struct f_malloc_block *)((uint8_t *)ptr - sizeof(struct f_malloc_block));
    if (block_valid(blk))
    {
        if (mutex_trylock(mlock) < 0) {
            /* Try again later. */
            tasklet_add(f_free, ptr);
            return;
        }

        if ((blk->flags & F_IN_USE) == 0) {
            task_segfault((uint32_t)ptr, 0, MEMFAULT_DOUBLEFREE);
        }

        blk->flags &= ~F_IN_USE;
        /* stats */
        f_malloc_stats[MEMPOOL(blk->flags)].free_calls++;
        f_malloc_stats[MEMPOOL(blk->flags)].objects_allocated--;
        f_malloc_stats[MEMPOOL(blk->flags)].mem_allocated -= (uint32_t)blk->size + sizeof(struct f_malloc_block);
        if ((blk->flags & MEM_TASK) == 0)
            blk_rearrange(blk);

        mutex_unlock(mlock);
    } else {
        dbg_malloc("FREE ERR: %p is not a valid allocated pointer!\n", blk);
    }
}

/* Some statistic helpers */

uint32_t mem_stats_frag(int pool)
{
    uint32_t frag_size = 0u;
    struct f_malloc_block *blk;
        
    mutex_lock(mlock);    
    blk = malloc_entry[pool];
    while (blk) {
        if (!in_use(blk)) 
            frag_size += blk->size + sizeof(struct f_malloc_block); 
        blk = blk->next;
    }
    mutex_unlock(mlock);
    return frag_size;
}


int fmalloc_owner(void *_ptr)
{
    struct f_malloc_block *blk;
    uint8_t *ptr = (uint8_t *)_ptr;
    blk = malloc_entry[MEM_USER];

    while(blk) {
        uint8_t *mem_start = (uint8_t *)blk + sizeof(struct f_malloc_block);
        uint8_t *mem_end   = mem_start + blk->size;


        if ( (ptr >= mem_start) && (ptr < mem_end) ) {
            if (block_valid(blk) && in_use(blk)) 
                return blk->pid;
            else 
                return -1;
        }
        blk = blk->next;
    }
    return -1;
}

int fmalloc_chown(void *ptr, uint16_t pid)
{
    struct f_malloc_block *blk = (struct f_malloc_block *) ( ((uint8_t *)ptr)  - sizeof(struct f_malloc_block));
    if (block_valid(blk))
        blk->pid = pid;
}

/* Syscalls back-end (for userspace memory call handling) */
void *sys_malloc_hdlr(int size)
{
    void *addr = f_malloc(MEM_USER, size);
    #ifdef CONFIG_SRAM_EXTRA
    if (!addr)
        addr = f_malloc(MEM_EXTRA, size);
    #endif
    return addr;
}

int sys_free_hdlr(int addr)
{
    f_free((void *)addr);
    return 0;
}

int sys_calloc_hdlr(int n, int size)
{
    return (int)f_calloc(MEM_USER, n, size);
}

int sys_realloc_hdlr(int addr, int size)
{
    if (task_ptr_valid((void *)addr))
        return -EACCES;
    return (int)f_realloc(MEM_USER, (void *)addr, size);
}

/*------------------*/
/* Test functions   */
/*------------------*/


#if defined __linux__ || defined _WIN32 /* test application */
    int task_segfault(uint32_t mem, uint32_t inst, int flags) {
        dbg_malloc("Memory violation\n");
        exit(1);
    }

    void print_malloc_stats(void)
    {
        dbg_malloc("\n=== FROSTED MALLOC STATS ===\n");
        dbg_malloc("--> malloc calls: %d\n", f_malloc_stats[0].malloc_calls);
        dbg_malloc("--> free   calls: %d\n", f_malloc_stats[0].free_calls);
        dbg_malloc("--> objects allocated: %d\n", f_malloc_stats[0].objects_allocated);
        dbg_malloc("--> memory  allocated: %d\n", f_malloc_stats[0].mem_allocated);
        dbg_malloc("=== FROSTED MALLOC STATS ===\n\n");
    }
    
    void print_malloc_entries(void)
    {
        struct f_malloc_block *blk = malloc_entry[0];
        uint32_t i = 0;
    
        /* See if we can find a free block that fits */
        while (blk) /* last entry will break the loop */
        {
            dbg_malloc(">>> Entry #%d: \n", i);
            dbg_malloc("    Address (blk) %p \n", blk);
            dbg_malloc("    Address (usr) %p \n", ((uint8_t*)blk) + sizeof(struct f_malloc_block));
            dbg_malloc("    Prev: %p \n", blk->prev);
            dbg_malloc("    Next: %p \n", blk->next);
            dbg_malloc("    Size (usr) %d \n", blk->size);
            dbg_malloc("    Flags: %08x \n", blk->flags);
            dbg_malloc("    Magic: 0x%08x \n", blk->magic);
            i++;
            blk = blk->next;
        }
    }

    int main(int argc, char ** argv)
    {
        void * test10 = f_malloc(0, 10);
        void * test200 = f_malloc(0, 200);
        void * test100 = NULL;
        dbg_malloc("test10: %p\n", test10);
        dbg_malloc("test200: %p\n", test200);
        f_free(test10);
        print_malloc_stats();
        print_malloc_entries();
    
        dbg_malloc("\nTrying to re-use freed memory + allocate more\n");
        test10 = f_malloc(0, 10); // this should re-use exisiting entry
        test100 = f_malloc(0, 100); // this should alloc more memory through sbrk
        print_malloc_stats();
        print_malloc_entries();
    
        dbg_malloc("\nFreeing all of the memory\n");
        f_free(test10);
        f_free(test200);
        f_free(test100);
        print_malloc_stats();
        print_malloc_entries();

        dbg_malloc("Trying to re-use freed memory\n");
        test100 = f_malloc(0, 100); // this should re-use memory in the freed pool
        print_malloc_stats();
        print_malloc_entries();
        f_free(test100);

        dbg_malloc("Allocating more memory\n");
        test10 = f_malloc(0, 10);
        test200 = f_malloc(0, 200);
        print_malloc_stats();
        print_malloc_entries();
        f_free(test10);
        f_free(test200);

        dbg_malloc("Test Realloc \n");
        test10 = f_malloc(0,10);
        test100 = f_malloc(0,110);
        test200 = f_malloc(0,1);
        if (!test10 || !test100 || !test200) {
            dbg_malloc("malloc failed!\n");
            exit(1);
        }
        print_malloc_stats();
        print_malloc_entries();
        test10 = f_realloc(0, test10, 210);
        test100 = f_realloc(0, test100, 2010);
        test200 = f_realloc(0, test200, 2);
        if (!test10 || !test100 || !test200) {
            dbg_malloc("realloc failed!\n");
            exit(1);
        }
        print_malloc_stats();
        print_malloc_entries();

        dbg_malloc("\nFreeing all of the memory\n");
        f_free(test10);
        f_free(test200);
        f_free(test100);
        print_malloc_stats();
        print_malloc_entries();
    
        return 0;
    }
#endif



