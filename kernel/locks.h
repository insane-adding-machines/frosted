#include "frosted.h"
/* Structures */
struct semaphore {
    int value;
    int listeners;
    struct task **listener;
};


int _mutex_lock(void *);
int _mutex_unlock(void *);

int _sem_wait(void *);
int _sem_post(void *);
