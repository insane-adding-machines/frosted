/* The file syscall_table.c is auto generated. DO NOT EDIT, CHANGES WILL BE LOST. */
/* If you want to add syscalls, use syscall_table_gen.py  */

#include "frosted.h"
#include "syscall_table.h"
/* Syscall: setclock(1 arguments) */
int sys_setclock(uint32_t arg1){
    syscall(SYS_SETCLOCK, arg1, 0, 0, 0, 0); 
}

/* Syscall: sleep(1 arguments) */
int sys_sleep(uint32_t arg1){
    syscall(SYS_SLEEP, arg1, 0, 0, 0, 0); 
}

/* Syscall: suspend(1 arguments) */
int sys_suspend(uint32_t arg1){
    syscall(SYS_SUSPEND, arg1, 0, 0, 0, 0); 
}

/* Syscall: thread_create(3 arguments) */
int sys_thread_create(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_THREAD_CREATE, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: thread_join(2 arguments) */
int sys_thread_join(uint32_t arg1, uint32_t arg2){
    syscall(SYS_THREAD_JOIN, arg1, arg2, 0, 0, 0); 
}

/* Syscall: test(5 arguments) */
int sys_test(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
    syscall(SYS_TEST, arg1, arg2, arg3, arg4, arg5); 
}

/* Syscall: getpid(0 arguments) */
int sys_getpid(void){
    syscall(SYS_GETPID, 0, 0, 0, 0, 0); 
}

/* Syscall: getppid(0 arguments) */
int sys_getppid(void){
    syscall(SYS_GETPPID, 0, 0, 0, 0, 0); 
}

/* Syscall: open(3 arguments) */
int sys_open(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_OPEN, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: close(1 arguments) */
int sys_close(uint32_t arg1){
    syscall(SYS_CLOSE, arg1, 0, 0, 0, 0); 
}

/* Syscall: read(3 arguments) */
int sys_read(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_READ, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: write(3 arguments) */
int sys_write(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_WRITE, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: seek(3 arguments) */
int sys_seek(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_SEEK, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: mkdir(2 arguments) */
int sys_mkdir(uint32_t arg1, uint32_t arg2){
    syscall(SYS_MKDIR, arg1, arg2, 0, 0, 0); 
}

/* Syscall: unlink(1 arguments) */
int sys_unlink(uint32_t arg1){
    syscall(SYS_UNLINK, arg1, 0, 0, 0, 0); 
}

/* Syscall: gettimeofday(1 arguments) */
int sys_gettimeofday(uint32_t arg1){
    syscall(SYS_GETTIMEOFDAY, arg1, 0, 0, 0, 0); 
}

/* Syscall: malloc(1 arguments) */
int sys_malloc(uint32_t arg1){
    syscall(SYS_MALLOC, arg1, 0, 0, 0, 0); 
}

/* Syscall: mem_init(1 arguments) */
int sys_mem_init(uint32_t arg1){
    syscall(SYS_MEM_INIT, arg1, 0, 0, 0, 0); 
}

/* Syscall: free(1 arguments) */
int sys_free(uint32_t arg1){
    syscall(SYS_FREE, arg1, 0, 0, 0, 0); 
}

/* Syscall: calloc(2 arguments) */
int sys_calloc(uint32_t arg1, uint32_t arg2){
    syscall(SYS_CALLOC, arg1, arg2, 0, 0, 0); 
}

/* Syscall: realloc(2 arguments) */
int sys_realloc(uint32_t arg1, uint32_t arg2){
    syscall(SYS_REALLOC, arg1, arg2, 0, 0, 0); 
}

/* Syscall: opendir(1 arguments) */
int sys_opendir(uint32_t arg1){
    syscall(SYS_OPENDIR, arg1, 0, 0, 0, 0); 
}

/* Syscall: readdir(2 arguments) */
int sys_readdir(uint32_t arg1, uint32_t arg2){
    syscall(SYS_READDIR, arg1, arg2, 0, 0, 0); 
}

/* Syscall: closedir(1 arguments) */
int sys_closedir(uint32_t arg1){
    syscall(SYS_CLOSEDIR, arg1, 0, 0, 0, 0); 
}

/* Syscall: stat(2 arguments) */
int sys_stat(uint32_t arg1, uint32_t arg2){
    syscall(SYS_STAT, arg1, arg2, 0, 0, 0); 
}

/* Syscall: poll(3 arguments) */
int sys_poll(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_POLL, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: ioctl(3 arguments) */
int sys_ioctl(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_IOCTL, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: link(2 arguments) */
int sys_link(uint32_t arg1, uint32_t arg2){
    syscall(SYS_LINK, arg1, arg2, 0, 0, 0); 
}

/* Syscall: chdir(1 arguments) */
int sys_chdir(uint32_t arg1){
    syscall(SYS_CHDIR, arg1, 0, 0, 0, 0); 
}

/* Syscall: getcwd(2 arguments) */
int sys_getcwd(uint32_t arg1, uint32_t arg2){
    syscall(SYS_GETCWD, arg1, arg2, 0, 0, 0); 
}

/* Syscall: sem_init(1 arguments) */
int sys_sem_init(uint32_t arg1){
    syscall(SYS_SEM_INIT, arg1, 0, 0, 0, 0); 
}

/* Syscall: sem_post(1 arguments) */
int sys_sem_post(uint32_t arg1){
    syscall(SYS_SEM_POST, arg1, 0, 0, 0, 0); 
}

/* Syscall: sem_wait(1 arguments) */
int sys_sem_wait(uint32_t arg1){
    syscall(SYS_SEM_WAIT, arg1, 0, 0, 0, 0); 
}

/* Syscall: sem_destroy(1 arguments) */
int sys_sem_destroy(uint32_t arg1){
    syscall(SYS_SEM_DESTROY, arg1, 0, 0, 0, 0); 
}

/* Syscall: mutex_init(0 arguments) */
int sys_mutex_init(void){
    syscall(SYS_MUTEX_INIT, 0, 0, 0, 0, 0); 
}

/* Syscall: mutex_unlock(1 arguments) */
int sys_mutex_unlock(uint32_t arg1){
    syscall(SYS_MUTEX_UNLOCK, arg1, 0, 0, 0, 0); 
}

/* Syscall: mutex_lock(1 arguments) */
int sys_mutex_lock(uint32_t arg1){
    syscall(SYS_MUTEX_LOCK, arg1, 0, 0, 0, 0); 
}

/* Syscall: mutex_destroy(1 arguments) */
int sys_mutex_destroy(uint32_t arg1){
    syscall(SYS_MUTEX_DESTROY, arg1, 0, 0, 0, 0); 
}

/* Syscall: socket(3 arguments) */
int sys_socket(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_SOCKET, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: bind(2 arguments) */
int sys_bind(uint32_t arg1, uint32_t arg2){
    syscall(SYS_BIND, arg1, arg2, 0, 0, 0); 
}

/* Syscall: accept(2 arguments) */
int sys_accept(uint32_t arg1, uint32_t arg2){
    syscall(SYS_ACCEPT, arg1, arg2, 0, 0, 0); 
}

/* Syscall: connect(2 arguments) */
int sys_connect(uint32_t arg1, uint32_t arg2){
    syscall(SYS_CONNECT, arg1, arg2, 0, 0, 0); 
}

/* Syscall: listen(2 arguments) */
int sys_listen(uint32_t arg1, uint32_t arg2){
    syscall(SYS_LISTEN, arg1, arg2, 0, 0, 0); 
}

/* Syscall: sendto(5 arguments) */
int sys_sendto(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
    syscall(SYS_SENDTO, arg1, arg2, arg3, arg4, arg5); 
}

/* Syscall: recvfrom(5 arguments) */
int sys_recvfrom(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
    syscall(SYS_RECVFROM, arg1, arg2, arg3, arg4, arg5); 
}

/* Syscall: setsockopt(5 arguments) */
int sys_setsockopt(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
    syscall(SYS_SETSOCKOPT, arg1, arg2, arg3, arg4, arg5); 
}

/* Syscall: getsockopt(5 arguments) */
int sys_getsockopt(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
    syscall(SYS_GETSOCKOPT, arg1, arg2, arg3, arg4, arg5); 
}

/* Syscall: shutdown(2 arguments) */
int sys_shutdown(uint32_t arg1, uint32_t arg2){
    syscall(SYS_SHUTDOWN, arg1, arg2, 0, 0, 0); 
}

/* Syscall: dup(1 arguments) */
int sys_dup(uint32_t arg1){
    syscall(SYS_DUP, arg1, 0, 0, 0, 0); 
}

/* Syscall: dup2(2 arguments) */
int sys_dup2(uint32_t arg1, uint32_t arg2){
    syscall(SYS_DUP2, arg1, arg2, 0, 0, 0); 
}

/* Syscall: mount(5 arguments) */
int sys_mount(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
    syscall(SYS_MOUNT, arg1, arg2, arg3, arg4, arg5); 
}

/* Syscall: umount(2 arguments) */
int sys_umount(uint32_t arg1, uint32_t arg2){
    syscall(SYS_UMOUNT, arg1, arg2, 0, 0, 0); 
}

/* Syscall: kill(2 arguments) */
int sys_kill(uint32_t arg1, uint32_t arg2){
    syscall(SYS_KILL, arg1, arg2, 0, 0, 0); 
}

/* Syscall: isatty(1 arguments) */
int sys_isatty(uint32_t arg1){
    syscall(SYS_ISATTY, arg1, 0, 0, 0, 0); 
}

/* Syscall: exec(2 arguments) */
int sys_exec(uint32_t arg1, uint32_t arg2){
    syscall(SYS_EXEC, arg1, arg2, 0, 0, 0); 
}

/* Syscall: ttyname_r(3 arguments) */
int sys_ttyname_r(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_TTYNAME_R, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: exit(1 arguments) */
int sys_exit(uint32_t arg1){
    syscall(SYS_EXIT, arg1, 0, 0, 0, 0); 
}

/* Syscall: tcsetattr(3 arguments) */
int sys_tcsetattr(uint32_t arg1, uint32_t arg2, uint32_t arg3){
    syscall(SYS_TCSETATTR, arg1, arg2, arg3, 0,  0); 
}

/* Syscall: tcgetattr(2 arguments) */
int sys_tcgetattr(uint32_t arg1, uint32_t arg2){
    syscall(SYS_TCGETATTR, arg1, arg2, 0, 0, 0); 
}

/* Syscall: tcsendbreak(2 arguments) */
int sys_tcsendbreak(uint32_t arg1, uint32_t arg2){
    syscall(SYS_TCSENDBREAK, arg1, arg2, 0, 0, 0); 
}

/* Syscall: pipe2(2 arguments) */
int sys_pipe2(uint32_t arg1, uint32_t arg2){
    syscall(SYS_PIPE2, arg1, arg2, 0, 0, 0); 
}

/* External handlers (defined elsewhere) : */ 
extern int sys_setclock_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_sleep_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_suspend_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_thread_create_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_thread_join_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_test_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_getpid_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_getppid_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_open_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_close_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_read_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_write_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_seek_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_mkdir_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_unlink_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_gettimeofday_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_malloc_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_mem_init_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_free_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_calloc_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_realloc_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_opendir_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_readdir_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_closedir_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_stat_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_poll_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_ioctl_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_link_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_chdir_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_getcwd_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_sem_init_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_sem_post_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_sem_wait_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_sem_destroy_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_mutex_init_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_mutex_unlock_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_mutex_lock_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_mutex_destroy_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_socket_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_bind_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_accept_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_connect_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_listen_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_sendto_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_recvfrom_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_setsockopt_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_getsockopt_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_shutdown_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_dup_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_dup2_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_mount_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_umount_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_kill_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_isatty_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_exec_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_ttyname_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_exit_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_tcsetattr_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_tcgetattr_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_tcsendbreak_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern int sys_pipe2_hdlr(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

void syscalls_init(void) {
	sys_register_handler(0, sys_setclock_hdlr);
	sys_register_handler(1, sys_sleep_hdlr);
	sys_register_handler(2, sys_suspend_hdlr);
	sys_register_handler(3, sys_thread_create_hdlr);
	sys_register_handler(4, sys_thread_join_hdlr);
	sys_register_handler(5, sys_test_hdlr);
	sys_register_handler(6, sys_getpid_hdlr);
	sys_register_handler(7, sys_getppid_hdlr);
	sys_register_handler(8, sys_open_hdlr);
	sys_register_handler(9, sys_close_hdlr);
	sys_register_handler(10, sys_read_hdlr);
	sys_register_handler(11, sys_write_hdlr);
	sys_register_handler(12, sys_seek_hdlr);
	sys_register_handler(13, sys_mkdir_hdlr);
	sys_register_handler(14, sys_unlink_hdlr);
	sys_register_handler(15, sys_gettimeofday_hdlr);
	sys_register_handler(16, sys_malloc_hdlr);
	sys_register_handler(17, sys_mem_init_hdlr);
	sys_register_handler(18, sys_free_hdlr);
	sys_register_handler(19, sys_calloc_hdlr);
	sys_register_handler(20, sys_realloc_hdlr);
	sys_register_handler(21, sys_opendir_hdlr);
	sys_register_handler(22, sys_readdir_hdlr);
	sys_register_handler(23, sys_closedir_hdlr);
	sys_register_handler(24, sys_stat_hdlr);
	sys_register_handler(25, sys_poll_hdlr);
	sys_register_handler(26, sys_ioctl_hdlr);
	sys_register_handler(27, sys_link_hdlr);
	sys_register_handler(28, sys_chdir_hdlr);
	sys_register_handler(29, sys_getcwd_hdlr);
	sys_register_handler(30, sys_sem_init_hdlr);
	sys_register_handler(31, sys_sem_post_hdlr);
	sys_register_handler(32, sys_sem_wait_hdlr);
	sys_register_handler(33, sys_sem_destroy_hdlr);
	sys_register_handler(34, sys_mutex_init_hdlr);
	sys_register_handler(35, sys_mutex_unlock_hdlr);
	sys_register_handler(36, sys_mutex_lock_hdlr);
	sys_register_handler(37, sys_mutex_destroy_hdlr);
	sys_register_handler(38, sys_socket_hdlr);
	sys_register_handler(39, sys_bind_hdlr);
	sys_register_handler(40, sys_accept_hdlr);
	sys_register_handler(41, sys_connect_hdlr);
	sys_register_handler(42, sys_listen_hdlr);
	sys_register_handler(43, sys_sendto_hdlr);
	sys_register_handler(44, sys_recvfrom_hdlr);
	sys_register_handler(45, sys_setsockopt_hdlr);
	sys_register_handler(46, sys_getsockopt_hdlr);
	sys_register_handler(47, sys_shutdown_hdlr);
	sys_register_handler(48, sys_dup_hdlr);
	sys_register_handler(49, sys_dup2_hdlr);
	sys_register_handler(50, sys_mount_hdlr);
	sys_register_handler(51, sys_umount_hdlr);
	sys_register_handler(52, sys_kill_hdlr);
	sys_register_handler(53, sys_isatty_hdlr);
	sys_register_handler(54, sys_exec_hdlr);
	sys_register_handler(55, sys_ttyname_hdlr);
	sys_register_handler(56, sys_exit_hdlr);
	sys_register_handler(57, sys_tcsetattr_hdlr);
	sys_register_handler(58, sys_tcgetattr_hdlr);
	sys_register_handler(59, sys_tcsendbreak_hdlr);
	sys_register_handler(60, sys_pipe2_hdlr);
}
