#ifndef INCLUDED_SCHEDULER_H
#define INCLUDED_SCHEDULER_H

int scheduler_exec(void (*init)(void *), void *args, uint32_t pic);

int scheduler_task_state(int pid);
unsigned scheduler_stack_used(int pid);



#endif
