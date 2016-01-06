#ifndef FROSTED_INCLUDED_H
#define FROSTED_INCLUDED_H

#define KERNEL
#include "frosted_api.h"
#include "malloc.h"
#include "interrupts.h"
#include "string.h"
#include "errno.h"

#define TASK_IDLE       0
#define TASK_RUNNABLE   1
#define TASK_RUNNING    2
#define TASK_WAITING    3
#define TASK_FORKED     4
#define TASK_ZOMBIE     0x66
#define TASK_OVER       0xFF

#define MEMFAULT_ACCESS 0x00
#define MEMFAULT_DOUBLEFREE 0x01

//#define DEBUG

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Types */
struct task;
struct fnode;
struct semaphore;
struct termios;
typedef struct semaphore sem_t;
typedef struct semaphore frosted_mutex_t;

/* generics */
volatile unsigned int jiffies;
volatile int _syscall_retval;
void frosted_init(void);
void klog_set_write(int (*wr)(int, const void *, unsigned int));

/* klog */
/* TODO: implement snprintf! */
#if 0
#define klog(priority, ...)   {                  \
    char *str = (char *)f_malloc(256);           \
    if (str) {                                   \
        if (priority <= KLOG_LEVEL) {            \
            snprintf(str, 256,  __VA_ARGS__ );   \
            klog_write(0, str, strlen(str));     \
        }                                        \
        f_free(str);                             \
    }                                            \
}
#else
#define klog(...) do{}while(0)
#endif



/* Systick & co. */
int _clock_interval;
void SysTick_Hook(void);
int SysTick_Interval(unsigned long interval);
void SysTick_on(void);
void SysTick_off(void);
int SysTick_interval(unsigned long interval);
void ktimer_init(void);

struct ktimer;
int ktimer_add(uint32_t count, void (*handler)(uint32_t, void *), void *arg);
void ktimer_cancel(struct ktimer *t);

/* Scheduler */
void frosted_scheduler_on(void);
int scheduler_exec(void (*init)(void *), void *arg);
uint16_t scheduler_get_cur_pid(void);
uint16_t scheduler_get_cur_ppid(void);
int task_timeslice(void);
int task_running(void);
int task_filedesc_add(struct fnode *f);
int task_fd_setmask(int fd, uint32_t mask);
uint32_t task_fd_getmask(int fd);
struct fnode *task_filedesc_get(int fd);
int task_segfault(uint32_t addr, uint32_t instr, int flags);

int task_fd_readable(int fd);
int task_fd_writable(int fd);
int task_filedesc_del(int fd);
void task_suspend(void);
void task_resume(int pid);
int task_create(void (*init)(void *), void *arg, unsigned int prio);
struct fnode *task_getcwd(void);
void task_chdir(struct fnode *f);

int sem_wait(sem_t *s);
int sem_post(sem_t *s);
sem_t *sem_init(int val);
int sem_destroy(sem_t *s);

int frosted_mutex_lock(frosted_mutex_t *s);
int frosted_mutex_unlock(frosted_mutex_t *s);
frosted_mutex_t *frosted_mutex_init();
int frostd_mutex_destroy(frosted_mutex_t *s);

#define schedule()   *((uint32_t volatile *)0xE000ED04) = 0x10000000 

/* Timers */
int Timer_on(unsigned int n);

/* Tasklets */
void tasklet_add(void (*exe)(void*), void *arg);
void check_tasklets(void);

/* Modules */
struct module *MODS;
int register_module(struct module *m);
int unregister_module(struct module *m);
struct module *module_search(char *name);

/* System */
void mpu_init(void);
int sys_register_handler(uint32_t n, int (*_sys_c)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t));
int syscall(uint32_t syscall_nr, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
void syscalls_init(void);

#define SYS_CALL_AGAIN (-1024) 

/* VFS */
void vfs_init(void);
int fno_fullpath(struct fnode *f, char *dst, int len);

#define FL_RDONLY 0x01
#define FL_WRONLY 0x02
#define FL_RDWR   (FL_RDONLY | FL_WRONLY)
#define FL_DIR    0x04
#define FL_INUSE  0x08
#define FL_TTY    0x10

#define FL_LINK   0x80
#define FL_EXEC   0xC0

struct fnode {
    struct module *owner;
    char *fname;
    char *linkname;
    uint32_t flags;
    struct fnode *parent;
    struct fnode *children;
    const void *priv;
    uint32_t size;
    uint32_t off;
    uint32_t usage;
    struct fnode *next;
};

#define FNO_MOD_PRIV(fno,mod) (((fno == NULL)?NULL:((mod != fno->owner)?NULL:(fno->priv))))

struct mountpoint 
{
    struct fnode *target;
    struct mountpoint *next;
};

struct fnode *fno_create(struct module *owner, const char *name, struct fnode *parent);
struct fnode *fno_create_rdonly(struct module *owner, const char *name, struct fnode *parent);
struct fnode *fno_create_wronly(struct module *owner, const char *name, struct fnode *parent);
struct fnode *fno_mkdir(struct module *owner, const char *name, struct fnode *parent);
void fno_unlink(struct fnode *fno);
struct fnode *fno_search(const char *path);
int vfs_symlink(char *file, char *link);

/* Modules (for files/sockets) */


#define FAMILY_UNIX 0x0000
#define FAMILY_INET 0x0001
#define FAMILY_DEV  0x0DEF
#define FAMILY_FILE 0xFFFF

#define MODNAME_SIZE 32


struct module {
    uint16_t family;
    char name[MODNAME_SIZE];

    /* If this is a filesystem related module, it should probably define these */
    int (*mount)(char *source, char *target, uint32_t flags, void *arg);
    int (*umount)(char *target, uint32_t flags);
    int (*mount_info)(struct fnode *fno, char *buf, int size);

    /* TODO: perhaps we should make a union here... */
    struct module_operations {
        /* Common module operations */
        int (*read) (struct fnode *fno, void *buf, unsigned int len);
        int (*write)(struct fnode *fno, const void *buf, unsigned int len);
        int (*poll) (struct fnode *fno, uint16_t events, uint16_t *revents);
        int (*close)(struct fnode *fno);
        int (*ioctl)(struct fnode *fno, const uint32_t cmd, void *arg);

        /* Files only (NULL == socket) */
        int (*open)(const char *path, int flags);
        int (*seek)(struct fnode *fno, int offset, int whence);
        int (*creat)(struct fnode *fno);
        int (*unlink)(struct fnode *fno);
        void * (*exe)(struct fnode *fno, void *arg);

        /* Sockets only (NULL == file) */
        int (*socket)(int domain, int type, int protocol);
        int (*recvfrom)(int fd, void *buf, unsigned int len, int flags, struct sockaddr *addr, unsigned int *addrlen);
        int (*sendto)(int fd, const void *buf, unsigned int len, int flags, struct sockaddr *addr, unsigned int addrlen);
        int (*bind)(int fd, struct sockaddr *addr, unsigned int addrlen);
        int (*accept)(int fd, struct sockaddr *addr, unsigned int *addrlen);
        int (*connect)(int fd, struct sockaddr *addr, unsigned int addrlen);
        int (*listen)(int fd, int backlog);
        int (*shutdown)(int fd, uint16_t how);
        int (*setsockopt)(int sd, int level, int optname, void *optval, unsigned int optlen);
        int (*getsockopt)(int sd, int level, int optname, void *optval, unsigned int *optlen);


        /* Terminal operations */
        int (*tcsetattr)(int td, int opts, const struct termios *tp);
        int (*tcgetattr)(int td, struct termios *tp);

    } ops;
    struct module *next;
};


void task_run(void);
void kernel_task_init(void);



#define kalloc(x) f_malloc(MEM_KERNEL,x)
#define task_space_alloc(x) f_malloc(MEM_TASK, x)
#define kcalloc(x,y) f_calloc(MEM_KERNEL,x,y)
#define krealloc(x,y) f_realloc(MEM_KERNEL,x,y)
#define kfree  f_free
#define task_space_free f_free

#endif /* BSP_INCLUDED_H */

