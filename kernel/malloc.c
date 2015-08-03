#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

/*------------------*/
/* Defines          */
/*------------------*/
#define F_MALLOC_MAGIC    (0xDECEA5ED) 

/*------------------*/
/* Structures       */
/*------------------*/
struct f_malloc_hdr {
    uint32_t magic; /* magic fingerprint */
    void * next;    /* next, or last block? */
    size_t size;    /* malloc size excluding this header - next header is adjacent, if !last_block */
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
static struct f_malloc_hdr * malloc_entry = NULL;
static struct f_malloc_stats f_malloc_stats = {};

/*------------------*/
/* Local functions  */
/*------------------*/
static struct f_malloc_hdr * f_find_first_fit(size_t size, struct f_malloc_hdr ** last)
{
    struct f_malloc_hdr *found = NULL, *hdr = malloc_entry;

    /* See if we can find a free block that fits */
    while (hdr) /* last entry will break the loop */
    {
        *last = hdr; /* last travelled node */

        if ((!hdr->in_use) && (hdr->size >= size))
        {
            /* found first fit! */
            found = hdr;
            break;
        } else {
            /* not free or no fit, continue */
            hdr = hdr->next;
        }
    }
    return found;
}

/*------------------*/
/* Public functions */
/*------------------*/
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
    struct f_malloc_hdr *hdr = malloc_entry;
    uint32_t i = 0;

    /* See if we can find a free block that fits */
    while (hdr) /* last entry will break the loop */
    {
        printf(">>> Entry #%d: \n", i);
        printf("    Address (hdr) %p \n", hdr);
        printf("    Address (usr) %p \n", ((uint8_t*)hdr) + sizeof(struct f_malloc_hdr));
        printf("    Size (usr) %d \n", hdr->size);
        printf("    In use? %d \n", hdr->in_use);
        printf("    Magic: 0x%08x \n", hdr->magic);
        i++;
        hdr = hdr->next;
    }
}

void * f_malloc(size_t size)
{
    struct f_malloc_hdr * hdr = NULL, *last = NULL;

    /* update stats */
    f_malloc_stats.malloc_calls++;

    /* Travel the linked list for first fit */
    hdr = f_find_first_fit(size, &last);
    if (hdr)
    {
        /* first fit found, now split it if neccesary */
        // XXX: If we start to split here, we should make a free that merges adjacent free sections, too
    } else {
        /* No first fit found: ask for new memory */
        hdr = (struct f_malloc_hdr *)sbrk(size + sizeof(struct f_malloc_hdr));  // can OS give us more memory?
        if ((long)hdr == -1)
            return NULL;

        /* first call -> set entrypoint */
        if (malloc_entry == NULL)
            malloc_entry = hdr;

        hdr->magic = F_MALLOC_MAGIC;
        hdr->size = size;
        hdr->next = NULL; /* Newly allocated memory is always last in the linked list */
        /* Link this entry to the previous last entry */
        if (last)
        {
            assert(last->next == NULL);
            last->next = hdr;
        }
    }

    /* destination found, fill in  meta-data */
    hdr->in_use = 1;

    /* update stats */
    f_malloc_stats.objects_allocated++;
    f_malloc_stats.mem_allocated += (uint32_t)size;

    return (void *)(((uint8_t *)hdr) + sizeof(struct f_malloc_hdr)); // pointer to newly allocated mem
}

void f_free(void * ptr)
{
    struct f_malloc_hdr * hdr;

    /* stats */
    f_malloc_stats.free_calls++;

    if (!ptr)
        return;

    hdr = (struct f_malloc_hdr *)((uint8_t *)ptr - sizeof(struct f_malloc_hdr));
    if (hdr->magic == F_MALLOC_MAGIC)
    {
        hdr->in_use = 0u;
        /* stats */
        f_malloc_stats.objects_allocated--;
        f_malloc_stats.mem_allocated -= (uint32_t)hdr->size;
        //XXX TODO: merge adjacent free blocks ?
    } else {
        printf("FREE ERR: %p is not a valid allocated pointer!\n", hdr);
    }
}


/*------------------*/
/* Test functions   */
/*------------------*/

#ifdef __linux__ /* test application */
int main(int argc, char ** argv)
{
    void * test = f_malloc(10);
    void * test2 = f_malloc(200);
    printf("test: %p\n", test);
    printf("test2: %p\n", test2);
    f_free(test);
    print_malloc_stats();
    print_malloc_entries();

    printf("Trying to re-use freed memory + allocate more\n");
    test = f_malloc(10); // this should re-use exisiting entry
    test2 = f_malloc(100); // this should alloc more memory through sbrk
    print_malloc_stats();
    print_malloc_entries();

    printf("Freeing some of the memory\n");
    f_free(test);
    f_free(test2);
    print_malloc_stats();
    print_malloc_entries();

    return 0;
}
#endif
