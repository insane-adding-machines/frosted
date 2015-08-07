#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

/*------------------*/
/* Defines          */
/*------------------*/
#define F_MALLOC_MAGIC    (0xDECEA5ED) 

#ifdef _WIN32
void * sbrk (int incr);
#endif

#define dbg_malloc printf

/*------------------*/
/* Structures       */
/*------------------*/
struct f_malloc_block {
    uint32_t magic;               /* magic fingerprint */
    struct f_malloc_block * prev;   /* previous block */
    struct f_malloc_block * next;   /* next, or last block? */
    size_t size;    /* malloc size excluding this block - next block is adjacent, if !last_block */
    int in_use;     /* free or not ? */
};

struct f_malloc_stats {
    uint32_t malloc_calls;
    uint32_t free_calls;
    uint32_t objects_allocated;
    uint32_t mem_allocated;
};

/*------------------*/
/* Local variables  */
/*------------------*/
static struct f_malloc_block * malloc_entry = NULL;
static struct f_malloc_stats f_malloc_stats = {};

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
    free_blk->in_use = 0u;
    return free_blk;
}

static struct f_malloc_block * f_find_first_fit(size_t size, struct f_malloc_block ** last)
{
    struct f_malloc_block *found = NULL, *blk = malloc_entry;

    /* See if we can find a free block that fits */
    while (blk) /* last entry will break the loop */
    {
        *last = blk; /* last travelled node */

        if ((!blk->in_use) && (blk->size >= size))
        {
            /* found first fit! */
            found = blk;
            break;
        } else {
            /* not free or no fit, continue */
            blk = blk->next;
        }
    }
    return found;
}

static struct f_malloc_block * f_find_best_fit(size_t size, struct f_malloc_block ** last)
{
    struct f_malloc_block *found = NULL, *blk = malloc_entry;

    /* See if we can find a free block that fits */
    while (blk) /* last entry will break the loop */
    {
        *last = blk; /* last travelled node */

        if ((!blk->in_use) && (blk->size >= size))
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

/*------------------*/
/* Public functions */
/*------------------*/

// XXX: TODO
void * f_calloc(size_t num, size_t size)
{
  return NULL;
}

// XXX: TODO
void* f_realloc(void* ptr, size_t size)
{
  return NULL;
}

void * f_malloc(size_t size)
{
    struct f_malloc_block * blk = NULL, *last = NULL;

    /* update stats */
    f_malloc_stats.malloc_calls++;

    /* Travel the linked list for first fit */
    blk = f_find_best_fit(size, &last);
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
        blk = (struct f_malloc_block *)sbrk(size + sizeof(struct f_malloc_block));  // can OS give us more memory?
        if ((long)blk == -1)
            return NULL;

        /* first call -> set entrypoint */
        if (malloc_entry == NULL)
        {
            malloc_entry = blk;
            blk->prev = NULL;
        }

        blk->magic = F_MALLOC_MAGIC;
        blk->size = size;
        blk->next = NULL; /* Newly allocated memory is always last in the linked list */
        /* Link this entry to the previous last entry */
        if (last)
        {
            assert(last->next == NULL);
            last->next = blk;
            blk->prev = last;
        }
    }

    /* destination found, fill in  meta-data */
    blk->in_use = 1;

    /* update stats */
    f_malloc_stats.objects_allocated++;
    f_malloc_stats.mem_allocated += (uint32_t)blk->size;

    return (void *)(((uint8_t *)blk) + sizeof(struct f_malloc_block)); // pointer to newly allocated mem
}

void f_free(void * ptr)
{
    struct f_malloc_block * blk;

    /* stats */
    f_malloc_stats.free_calls++;

    if (!ptr)
        return;

    blk = (struct f_malloc_block *)((uint8_t *)ptr - sizeof(struct f_malloc_block));
    if (blk->magic == F_MALLOC_MAGIC)
    {
        blk->in_use = 0u;
        /* stats */
        f_malloc_stats.objects_allocated--;
        f_malloc_stats.mem_allocated -= (uint32_t)blk->size;
        /* Merge adjecent free blocks */
        //XXX: How to check if actually adjacent? -> Pointer arithmetic? (sizeof(struct) + blk->size) ?
        // Or just assume for now that they are adjacent? -> Like FreeRTOS heap4
        if ((blk->prev) && (!blk->prev->in_use))
        {
            blk = merge_blocks(blk->prev, blk);
        }
        if ((blk->next) && (!blk->next->in_use))
        {
            blk = merge_blocks(blk, blk->next);
        }
    } else {
        dbg_malloc("FREE ERR: %p is not a valid allocated pointer!\n", blk);
    }
}

int sys_malloc_hdlr(int size)
{
    return (int)f_malloc(size);
}

int sys_free_hdlr(int addr)
{
    f_free((void *)addr);
    return 0;
}


/*------------------*/
/* Test functions   */
/*------------------*/

#if defined _WIN32
    uint8_t f_heap[32 * 1024];
    
    void * sbrk (int incr)
    {
      #define end f_heap
      //extern char   end; /* Set by linker.  */
      static char * heap_end;
      char *        prev_heap_end;
    
      if (heap_end == 0)
        heap_end = (char *)&end;
    
      prev_heap_end = heap_end;
      heap_end += incr;
    
      return (void *) prev_heap_end;
      #undef end
    }
#endif

#if defined __linux__ || defined _WIN32 /* test application */
    void print_malloc_stats(void)
    {
        printf("\n=== FROSTED MALLOC STATS ===\n");
        printf("--> malloc calls: %d\n", f_malloc_stats.malloc_calls);
        printf("--> free   calls: %d\n", f_malloc_stats.free_calls);
        printf("--> objects allocated: %d\n", f_malloc_stats.objects_allocated);
        printf("--> memory  allocated: %d\n", f_malloc_stats.mem_allocated);
        printf("=== FROSTED MALLOC STATS ===\n\n");
    }
    
    void print_malloc_entries(void)
    {
        struct f_malloc_block *blk = malloc_entry;
        uint32_t i = 0;
    
        /* See if we can find a free block that fits */
        while (blk) /* last entry will break the loop */
        {
            printf(">>> Entry #%d: \n", i);
            printf("    Address (blk) %p \n", blk);
            printf("    Address (usr) %p \n", ((uint8_t*)blk) + sizeof(struct f_malloc_block));
            printf("    Prev: %p \n", blk->prev);
            printf("    Next: %p \n", blk->next);
            printf("    Size (usr) %d \n", blk->size);
            printf("    In use? %d \n", blk->in_use);
            printf("    Magic: 0x%08x \n", blk->magic);
            i++;
            blk = blk->next;
        }
    }

    int main(int argc, char ** argv)
    {
        void * test10 = f_malloc(10);
        void * test200 = f_malloc(200);
        void * test100 = NULL;
        printf("test10: %p\n", test10);
        printf("test200: %p\n", test200);
        f_free(test10);
        print_malloc_stats();
        print_malloc_entries();
    
        printf("\nTrying to re-use freed memory + allocate more\n");
        test10 = f_malloc(10); // this should re-use exisiting entry
        test100 = f_malloc(100); // this should alloc more memory through sbrk
        print_malloc_stats();
        print_malloc_entries();
    
        printf("\nFreeing all of the memory\n");
        f_free(test10);
        f_free(test200);
        f_free(test100);
        print_malloc_stats();
        print_malloc_entries();

        printf("Trying to re-use freed memory\n");
        test100 = f_malloc(100); // this should re-use memory in the freed pool
        print_malloc_stats();
        print_malloc_entries();

        printf("Allocating more memory\n");
        test10 = f_malloc(10);
        test200 = f_malloc(200);
        print_malloc_stats();
        print_malloc_entries();
    
        return 0;
    }
#endif



