/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
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
#include "frosted.h"

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

/*------------------*/
/* Structures       */
/*------------------*/
struct f_malloc_block {
    uint32_t magic;               /* magic fingerprint */
    struct f_malloc_block * prev;   /* previous block */
    struct f_malloc_block * next;   /* next, or last block? */
    size_t size;    /* malloc size excluding this block - next block is adjacent, if !last_block */
    uint32_t flags; 
};


/*------------------*/
/* Local variables  */
/*------------------*/
static struct f_malloc_block * malloc_entry_kernel = NULL;
static struct f_malloc_block * malloc_entry_user = NULL;

struct f_malloc_stats f_malloc_stats[2] = {};

static const uint32_t kmem_size = (CONFIG_KMEM_SIZE << 10);

/*------------------*/
/* Local functions  */
/*------------------*/

/* merges two blocks
 *  first: must have lowest address 
 *  second: must have highest address 
 *  returns: pointer to the merged block
 */
static struct f_malloc_block * merge_blocks(struct f_malloc_block * first, struct f_malloc_block * second)
{
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
    if (blk->size - sizeof(struct f_malloc_block) < size)
        return NULL;
  
    /* shrink the block to requested size */
    free_size = blk->size - sizeof(struct f_malloc_block) - size;
    blk->size = size;
    /* create new block */
    free_blk = (struct f_malloc_block *)(((uint8_t *)blk) + sizeof(struct f_malloc_block) + (uint8_t)blk->size);
    free_blk->prev = blk;
    free_blk->next = blk->next;
    blk->next = free_blk;
    free_blk->magic = 0xDECEA5ED;
    free_blk->size = free_size;
    free_blk->flags = 0u;
    return free_blk;
}

static struct f_malloc_block * f_find_best_fit(int user, size_t size, struct f_malloc_block ** last)
{
    struct f_malloc_block *found = NULL, *blk = malloc_entry_kernel;

    if (user)
        blk = malloc_entry_user;

    /* See if we can find a free block that fits */
    while (blk) /* last entry will break the loop */
    {
        *last = blk; /* last travelled node */

        if ((!in_use(blk)) && (blk->size >= size))
        {
            /* found a fit - is it better than a possible previously found block? */
            if ((!found) || (blk->size < found->size))
                found = blk;
        }
        /* travel next block */
        blk = blk->next;
    }
    return found;
}

static void * f_sbrk(int user, int incr)
{
    extern char   end;           /* Set by linker */
    static char * heap_end_kernel;
    static char * heap_end_user;
    char *        prev_heap_end;

    if (heap_end_kernel == 0) {
        heap_end_kernel = &end;
        heap_end_user = ( &end ) + kmem_size;
    }

    if (user) {
        prev_heap_end = heap_end_user;
        heap_end_user += incr;
    } else {
        if ((heap_end_kernel + incr) > ((&end) + kmem_size))
            return (void*)(0 - 1);
        prev_heap_end = heap_end_kernel;
        heap_end_kernel += incr;
    }
    return (void *) prev_heap_end;
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
    void * out;
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
    else if (blk->magic == 0xDECEA5ED)
    {
        /* copy over old block, if valid pointer */
        size_t new_size, copy_size;
        if (size > blk->size)
        {
            new_size = size;
            copy_size = blk->size;
        } else {
            new_size = blk->size;
            copy_size = size;
        }
        out = f_malloc(flags, size);

        copy_size = (size > blk->size) ? blk->size : size; // copy smallest of two
        memcpy(out, ptr, copy_size);
    }

realloc_free:
    f_free(ptr);
    return out;
}

void * f_malloc(int flags, size_t size)
{
    struct f_malloc_block * blk = NULL, *last = NULL;
    int user = 0;

    if (flags & MEM_USER) {
        user = 1;
    }

    while((size % 4) != 0) {
        size++;
    } 

    /* update stats */
    f_malloc_stats[flags & MEM_USER].malloc_calls++;

    /* Travel the linked list for first fit */
    blk = f_find_best_fit(user, size, &last);
    if (blk)
    {
        dbg_malloc("Found best fit!\n");
        /* first fit found, now split it if it's much bigger than needed */
        if (size + (2*sizeof(struct f_malloc_block)) < blk->size)
        {
            dbg_malloc("Splitting blocks, since requested size [%d] << best fit block size [%d]!\n", size, blk->size);
            split_block(blk, size);
        }
    } else {
        /* No first fit found: ask for new memory */
        blk = (struct f_malloc_block *)f_sbrk(user, size + sizeof(struct f_malloc_block));  // can OS give us more memory?
        if ((long)blk == -1)
            return NULL;

        /* first call -> set entrypoint */
        if ((user) && (malloc_entry_user == NULL))
        {
            malloc_entry_user = blk;
            blk->prev = NULL;
        }
        if ((!user) && (malloc_entry_kernel == NULL))
        {
            malloc_entry_kernel = blk;
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
            blk->prev = last;
        }
    }

    /* destination found, fill in  meta-data */
    blk->flags = F_IN_USE | flags;

    /* update stats */
    f_malloc_stats[flags & MEM_USER].objects_allocated++;
    f_malloc_stats[flags & MEM_USER].mem_allocated += (uint32_t)blk->size;

    return (void *)(((uint8_t *)blk) + sizeof(struct f_malloc_block)); // pointer to newly allocated mem
}

void f_free(void * ptr)
{
    struct f_malloc_block * blk;

    /* stats */

    if (!ptr) {
        return;
    }

    blk = (struct f_malloc_block *)((uint8_t *)ptr - sizeof(struct f_malloc_block));
    if (blk->magic == F_MALLOC_MAGIC)
    {
        blk->flags = 0u;
        /* stats */
        f_malloc_stats[blk->flags & MEM_USER].free_calls++;
        f_malloc_stats[blk->flags & MEM_USER].objects_allocated--;
        f_malloc_stats[blk->flags & MEM_USER].mem_allocated -= (uint32_t)blk->size;
        /* Merge adjecent free blocks */
        //XXX: How to check if actually adjacent? -> Pointer arithmetic? (sizeof(struct) + blk->size) ?
        // Or just assume for now that they are adjacent? -> Like FreeRTOS heap4
        if ((blk->prev) && (!in_use(blk->prev)))
        {
            blk = merge_blocks(blk->prev, blk);
        }
        if ((blk->next) && (!in_use(blk->next)))
        {
            blk = merge_blocks(blk, blk->next);
        }
    } else {
        dbg_malloc("FREE ERR: %p is not a valid allocated pointer!\n", blk);
    }
}


/* Syscalls back-end (for userspace memory call handling) */
int sys_malloc_hdlr(int size)
{
    return (int)f_malloc(MEM_USER, size);
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
    return (int)f_realloc(MEM_USER, (void *)addr, size);
}


/*------------------*/
/* Test functions   */
/*------------------*/


#if defined __linux__ || defined _WIN32 /* test application */
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
        struct f_malloc_block *blk = malloc_entry_kernel;
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
        void * test10 = f_malloc(10);
        void * test200 = f_malloc(200);
        void * test100 = NULL;
        dbg_malloc("test10: %p\n", test10);
        dbg_malloc("test200: %p\n", test200);
        f_free(test10);
        print_malloc_stats();
        print_malloc_entries();
    
        dbg_malloc("\nTrying to re-use freed memory + allocate more\n");
        test10 = f_malloc(10); // this should re-use exisiting entry
        test100 = f_malloc(100); // this should alloc more memory through sbrk
        print_malloc_stats();
        print_malloc_entries();
    
        dbg_malloc("\nFreeing all of the memory\n");
        f_free(test10);
        f_free(test200);
        f_free(test100);
        print_malloc_stats();
        print_malloc_entries();

        dbg_malloc("Trying to re-use freed memory\n");
        test100 = f_malloc(100); // this should re-use memory in the freed pool
        print_malloc_stats();
        print_malloc_entries();

        dbg_malloc("Allocating more memory\n");
        test10 = f_malloc(10);
        test200 = f_malloc(200);
        print_malloc_stats();
        print_malloc_entries();
    
        return 0;
    }
#endif



