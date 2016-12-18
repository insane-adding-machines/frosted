#include "frosted.h"
/* Structures */
struct semaphore {
    int value;
    uint32_t signature;
    int listeners;
    int last;
    struct task **listener;
};


int suspend_on_sem_wait(sem_t *s);
int suspend_on_mutex_lock(mutex_t *s);
