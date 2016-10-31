
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DECLARE_HEAP(type, orderby)                                                     \
struct heap_element_##type {                                                            \
    uint32_t id;                                                                        \
    type data;                                                                          \
};                                                                                      \
struct heap_##type {                                                                    \
    uint32_t size;                                                                      \
    uint32_t n;                                                                         \
    uint32_t last_id;                                                                   \
    struct heap_element_##type *top;                                                    \
};                                                                                      \
typedef struct heap_##type heap_##type;                                                 \
static uint32_t _heap_idx_count = 0;                                                    \
static inline int heap_insert(struct heap_##type *heap, type *el)                       \
{                                                                                       \
    int i;                                                                              \
    struct heap_element_##type etmp;                                                    \
    memcpy(&etmp.data, el, sizeof(type));                                               \
    if (++heap->n >= heap->size) {                                                      \
        heap->top = krealloc(heap->top,                                                 \
                (heap->n + 1) * sizeof(struct heap_element_##type));                    \
        if (!heap->top) {                                                               \
            heap->n--;                                                                  \
            return -1;                                                                  \
        }                                                                               \
        heap->size++;                                                                   \
    }                                                                                   \
    etmp.id = heap->last_id++;                                                          \
    if ((heap->last_id & 0x80000000U) != 0)                                             \
       heap->last_id = 0; /* Wrap around */                                             \
    if (heap->n == 1) {                                                                 \
        memcpy(&heap->top[1], &etmp, sizeof(struct heap_element_##type));               \
        return 0;                                                                       \
    }                                                                                   \
    for (i = heap->n; ((i > 1) &&                                                       \
                (heap->top[i / 2].data.orderby > el->orderby)); i /= 2) {               \
        memcpy(&heap->top[i], &heap->top[i / 2], sizeof(struct heap_element_##type));   \
    }                                                                                   \
    memcpy(&heap->top[i], &etmp, sizeof(struct heap_element_##type));                   \
    return (int)etmp.id;                                                                \
} \
static inline int heap_peek(struct heap_##type *heap, type *first)                      \
{                                                                                       \
    type *ptr = NULL;                                                                   \
    struct heap_element_##type *last;                                                   \
    int i, child, ret;                                                                  \
    if(heap->n == 0) {                                                                  \
        return -1;                                                                      \
    }                                                                                   \
    memcpy(first, &heap->top[1].data, sizeof(type));                                    \
    last = &heap->top[heap->n--];                                                       \
    for(i = 1; (i * 2) <= heap->n; i = child) {                                         \
        child = 2 * i;                                                                  \
        if ((child != heap->n) &&                                                       \
            (heap->top[child + 1]).data.orderby                                         \
            < (heap->top[child]).data.orderby)                                          \
            child++;                                                                    \
        if (last->data.orderby >                                                        \
            heap->top[child].data.orderby)                                              \
            memcpy(&heap->top[i], &heap->top[child],                                    \
                    sizeof(struct heap_element_##type));                                \
        else                                                                            \
            break;                                                                      \
    }                                                                                   \
    memcpy(&heap->top[i], last, sizeof(struct heap_element_##type));                    \
    return 0;                                                                           \
} \
static inline int heap_delete(struct heap_##type *heap, int id)                         \
{                                                                                       \
    int found = 0;                                                                      \
    int i, child;                                                                       \
    struct heap_element_##type *last, temp;                                             \
    if (heap->n == 0) {                                                                 \
        return -1;                                                                      \
    }                                                                                   \
    for (i = 1; i <= heap->n; i++) {                                                    \
        if (heap->top[i].id == id) {                                                    \
                found = i;                                                              \
                break;                                                                  \
        }                                                                               \
    }                                                                                   \
    if (!found) {                                                                       \
        return -1;                                                                      \
    }                                                                                   \
    if (found == 1) {                                                                   \
        (void)heap_peek(heap, &temp.data);                                              \
        return 0;                                                                       \
    }                                                                                   \
    if (found == heap->n) {                                                             \
        heap->n--;                                                                      \
        return 0;                                                                       \
    }                                                                                   \
    last = &heap->top[heap->n--];                                                       \
    memcpy(&heap->top[found], last, sizeof(struct heap_element_##type));                \
    for(i = found; i < heap->n; i++) {                                                  \
        if (heap->top[i].data.orderby > heap->top[i + 1].data.orderby) {                \
            memcpy(&temp, &heap->top[i], sizeof(struct heap_element_##type));           \
            memcpy(&heap->top[i], &heap->top[i+1], sizeof(struct heap_element_##type)); \
            memcpy(&heap->top[i+1], &temp, sizeof(struct heap_element_##type));         \
        }                                                                               \
    }                                                                                   \
    return 0;                                                                           \
} \
static inline type *heap_first(heap_##type *heap)                                       \
{                                                                                       \
    if (heap->n == 0)                                                                   \
        return NULL;                                                                    \
    return &heap->top[1].data;                                                          \
} \
static inline heap_##type *heap_init(void)                                              \
{                                                                                       \
    heap_##type *p = kcalloc(1, sizeof(heap_##type));                                    \
    return p;                                                                           \
} \
static inline void heap_destroy(heap_##type *h)                                         \
{                                                                                       \
    kfree(h->top);                                                                      \
    kfree(h);                                                                           \
}

