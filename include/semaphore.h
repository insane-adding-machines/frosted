#ifndef SEM_INC
#define SEM_INC

sem_t *sem_init(int val);
int sem_post(sem_t *s);
int sem_wait(sem_t *s);
int sem_destroy(sem_t *s);

#endif

