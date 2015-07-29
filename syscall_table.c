/* The file syscall_table.c is auto generated. DO NOT EDIT, CHANGES WILL BE LOST. */
/* If you want to add syscalls, use syscall_table_gen.py  */

#include "frosted.h"
#include "syscall_table.h"
/* Syscall: setclock(1 arguments) */
int sys_setclock(uint32_t arg1){
    syscall(SYS_SETCLOCK, arg1, 0, 0, 0, 0); 
}

/* Syscall: start(0 arguments) */
int sys_start(void){
    syscall(SYS_START, 0, 0, 0, 0, 0); 
}

/* Syscall: stop(0 arguments) */
int sys_stop(void){
    syscall(SYS_STOP, 0, 0, 0, 0, 0); 
}

/* Syscall: sleep(1 arguments) */
int sys_sleep(uint32_t arg1){
    syscall(SYS_SLEEP, arg1, 0, 0, 0, 0); 
}

/* Syscall: thread_create(3 arguments) */
int sys_thread_create(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_THREAD_CREATE, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: test(5 arguments) */
int sys_test(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
    syscall(SYS_TEST, arg1, arg2, arg3, arg4, arg5); 
}

/* Syscall: open(3 arguments) */
int sys_open(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_OPEN, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: close(1 arguments) */
int sys_close(uint32_t arg1){
    syscall(SYS_CLOSE, arg1, 0, 0, 0, 0); 
}

/* External handlers (defined elsewhere) : */ 
extern sys_setclock_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern sys_start_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern sys_stop_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern sys_sleep_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern sys_thread_create_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern sys_test_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern sys_open_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern sys_close_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

void syscalls_init(void) {
	sys_register_handler(0, sys_setclock_hdlr);
	sys_register_handler(1, sys_start_hdlr);
	sys_register_handler(2, sys_stop_hdlr);
	sys_register_handler(3, sys_sleep_hdlr);
	sys_register_handler(4, sys_thread_create_hdlr);
	sys_register_handler(5, sys_test_hdlr);
	sys_register_handler(6, sys_open_hdlr);
	sys_register_handler(7, sys_close_hdlr);
}
