/* General use syscall.h file.
   The more ports that use this file, the simpler sim/common/nltvals.def
   remains.  */

#ifndef LIBGLOSS_SYSCALL_H
#define LIBGLOSS_SYSCALL_H

/* Note: This file may be included by assembler source.  */

/* These should be as small as possible to allow a port to use a trap type
   instruction, which the system call # as the trap (the d10v for instance
   supports traps 0..31).  An alternative would be to define one trap for doing
   system calls, and put the system call number in a register that is not used
   for the normal calling sequence (so that you don't have to shift down the
   arguments to add the system call number).  Obviously, if these system call
   numbers are ever changed, all of the simulators and potentially user code
   will need to be updated.  */

/* There is no current need for the following: SYS_execv, SYS_creat, SYS_wait,
   etc. etc.  Don't add them.  */

/* The file syscall_table.h is auto generated. DO NOT EDIT, CHANGES WILL BE LOST. */
/* If you want to add syscalls, use syscall_table_gen.py  */

#define SYS_setclock 			(0)
#define SYS_sleep 			(1)
#define SYS_suspend 			(2)
#define SYS_thread_create 			(3)
#define SYS_thread_join 			(4)
#define SYS_test 			(5)
#define SYS_getpid 			(6)
#define SYS_getppid 			(7)
#define SYS_open 			(8)
#define SYS_close 			(9)
#define SYS_read 			(10)
#define SYS_write 			(11)
#define SYS_seek 			(12)
#define SYS_mkdir 			(13)
#define SYS_unlink 			(14)
#define SYS_gettimeofday 			(15)
#define SYS_malloc 			(16)
#define SYS_mem_init 			(17)
#define SYS_free 			(18)
#define SYS_calloc 			(19)
#define SYS_realloc 			(20)
#define SYS_opendir 			(21)
#define SYS_readdir 			(22)
#define SYS_closedir 			(23)
#define SYS_stat 			(24)
#define SYS_poll 			(25)
#define SYS_ioctl 			(26)
#define SYS_chdir 			(27)
#define SYS_getcwd 			(28)
#define SYS_sem_init 			(29)
#define SYS_sem_post 			(30)
#define SYS_sem_wait 			(31)
#define SYS_sem_destroy 			(32)
#define SYS_mutex_init 			(33)
#define SYS_mutex_unlock 			(34)
#define SYS_mutex_lock 			(35)
#define SYS_mutex_destroy 			(36)
#define SYS_socket 			(37)
#define SYS_bind 			(38)
#define SYS_accept 			(39)
#define SYS_connect 			(40)
#define SYS_listen 			(41)
#define SYS_sendto 			(42)
#define SYS_recvfrom 			(43)
#define SYS_setsockopt 			(44)
#define SYS_getsockopt 			(45)
#define SYS_shutdown 			(46)
#define SYS_dup 			(47)
#define SYS_dup2 			(48)
#define SYS_kill 			(49)
#define SYS_exit 			(50)
#define _SYSCALLS_NR (51) /* We have 51 syscalls! */

#endif

