#ifndef INCLUDED_SCHEDULER_H
#define INCLUDED_SCHEDULER_H

int scheduler_exec(void (*init)(void *), void *args);

int scheduler_task_state(int pid);
unsigned scheduler_stack_used(int pid);
int task_ptr_valid(void *ptr);



#endif
