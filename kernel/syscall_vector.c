#include <stdint.h>
/* Syscall table Vector array */ 
extern int sys_setclock( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_sleep( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_suspend( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_thread_create( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_thread_join( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_test( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_getpid( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_getppid( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_open( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_close( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_read( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_write( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_seek( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_mkdir( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_unlink( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_gettimeofday( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_malloc( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_mem_init( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_free( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_calloc( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_realloc( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_opendir( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_readdir( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_closedir( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_stat( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_poll( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_ioctl( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_link( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_chdir( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_getcwd( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_sem_init( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_sem_post( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_sem_wait( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_sem_destroy( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_mutex_init( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_mutex_unlock( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_mutex_lock( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_mutex_destroy( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_socket( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_bind( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_accept( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_connect( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_listen( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_sendto( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_recvfrom( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_setsockopt( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_getsockopt( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_shutdown( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_dup( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_dup2( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_mount( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_umount( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_kill( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_exec( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
extern int sys_exit( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );
int __attribute__((used,section(".syscall_vector"))) (* const _k__syscall__[55])( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t ) = {
	sys_setclock,
	sys_sleep,
	sys_suspend,
	sys_thread_create,
	sys_thread_join,
	sys_test,
	sys_getpid,
	sys_getppid,
	sys_open,
	sys_close,
	sys_read,
	sys_write,
	sys_seek,
	sys_mkdir,
	sys_unlink,
	sys_gettimeofday,
	sys_malloc,
	sys_mem_init,
	sys_free,
	sys_calloc,
	sys_realloc,
	sys_opendir,
	sys_readdir,
	sys_closedir,
	sys_stat,
	sys_poll,
	sys_ioctl,
	sys_link,
	sys_chdir,
	sys_getcwd,
	sys_sem_init,
	sys_sem_post,
	sys_sem_wait,
	sys_sem_destroy,
	sys_mutex_init,
	sys_mutex_unlock,
	sys_mutex_lock,
	sys_mutex_destroy,
	sys_socket,
	sys_bind,
	sys_accept,
	sys_connect,
	sys_listen,
	sys_sendto,
	sys_recvfrom,
	sys_setsockopt,
	sys_getsockopt,
	sys_shutdown,
	sys_dup,
	sys_dup2,
	sys_mount,
	sys_umount,
	sys_kill,
	sys_exec,
	sys_exit
};
