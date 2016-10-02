#ifndef INCLUDED_SCHEDULER_H
#define INCLUDED_SCHEDULER_H
#include "vfs.h"

int scheduler_exec(struct vfs_info *v, void *args);

int scheduler_task_state(int pid);
unsigned scheduler_stack_used(int pid);
int task_ptr_valid(void *ptr);



#endif
