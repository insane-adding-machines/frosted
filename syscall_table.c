/* The file syscall_table.c is auto generated. DO NOT EDIT, CHANGES WILL BE LOST. */
/* If you want to add syscalls, use syscall_table_gen.py  */

#include "frosted.h"
#include "syscall_table.h"


/* Syscall: setclock(1 arguments) */
int sys_setclock(uint32_t arg1){
    syscall(SYS_SETCLOCK, arg1, 0, 0, 0, 0); 
}

int __attribute__((weak)) _sys_setclock(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
   return -1;
}



/* Syscall: start(0 arguments) */
int sys_start(void){
    syscall(SYS_START, 0, 0, 0, 0, 0); 
}

int __attribute__((weak)) _sys_start(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
   return -1;
}



/* Syscall: stop(0 arguments) */
int sys_stop(void){
    syscall(SYS_STOP, 0, 0, 0, 0, 0); 
}

int __attribute__((weak)) _sys_stop(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
   return -1;
}



/* Syscall: sleep(1 arguments) */
int sys_sleep(uint32_t arg1){
    syscall(SYS_SLEEP, arg1, 0, 0, 0, 0); 
}

int __attribute__((weak)) _sys_sleep(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
   return -1;
}



/* Syscall: thread_create(3 arguments) */
int sys_thread_create(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_THREAD_CREATE, arg1, arg2, arg3, 0,  0); 
}

int __attribute__((weak)) _sys_thread_create(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
   return -1;
}



/* Syscall: test(5 arguments) */
int sys_test(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
    syscall(SYS_TEST, arg1, arg2, arg3, arg4, arg5); 
}

int __attribute__((weak)) _sys_test(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
   return -1;
}

