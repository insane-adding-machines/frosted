#ifndef FROSTED_INCLUDED_H
#define FROSTED_INCLUDED_H

#define KERNEL
#include "frosted_api.h"
#include "malloc.h"
#include "interrupts.h"
#include "string.h"
#include "errno.h"
#include "vfs.h"
#include "kprintf.h"
#include "mpu.h"

#define TASK_IDLE       0
#define TASK_RUNNABLE   1
#define TASK_RUNNING    2
#define TASK_WAITING    3
#define TASK_FORKED     4
#define TASK_STOPPED    5
#define TASK_ZOMBIE     0x66
#define TASK_OVER       0xFF

#define MEMFAULT_ACCESS 0x00
#define MEMFAULT_DOUBLEFREE 0x01

//#define DEBUG

#include <stdint.h>
#include <string.h>
#include <sys/time.h>


/* Types */
struct task;
struct fnode;
struct semaphore;
struct termios;
typedef struct semaphore sem_t;
typedef struct semaphore mutex_t;

typedef uint32_t sigset_t;

/* generics */
volatile unsigned int jiffies;
volatile unsigned int rt_offset;
volatile int _syscall_retval;

/* Mach-specific initialization */
int machine_init(void);

/* Systick & co. */
int _clock_interval;
void SysTick_Hook(void);
int SysTick_Interval(unsigned long interval);
void SysTick_on(void);
void SysTick_off(void);
int SysTick_interval(unsigned long interval);
void ktimer_init(void);
int fpb_init(void);

struct ktimer;
int ktimer_add(uint32_t count, void (*handler)(uint32_t, void *), void *arg);
int ktimer_del(int tid);
void ktimer_cancel(struct ktimer *t);

/* FS initializers */
void memfs_init(void);
struct sysfs_fnode {
    struct fnode *fnode;
    int (*do_read)(struct sysfs_fnode *sfs, void *buf, int len);
    int (*do_write)(struct sysfs_fnode *sfs, const void *buf, int len);
};
void sysfs_init(void);


/* Scheduler */
#define NICE_DEFAULT (0)
#define NICE_RT (0 - 20)
#define NICE_MAX (20)

/* Called by boot code.
 * Set system NVIC priorities and start systick. */
void frosted_scheduler_on(void);

/* Forced preemption */
void task_preempt(void);
void task_preempt_all(void);

/* Execute/terminate a process. */
int scheduler_exec(struct vfs_info *v, void *args);
int task_create(struct vfs_info *vfsi, void *arg, unsigned int prio);
int task_kill(int pid, int signal);

/* Getter functions. XXX refactor to thread id XXX */
char * scheduler_task_name(int pid);
int scheduler_task_state(int pid);
unsigned scheduler_stack_used(int pid);
int scheduler_get_nice(int pid);

/* Get the task object for the current task */
struct task *this_task(void);
uint16_t this_task_getpid(void);

/* Resume target task */
void task_resume(struct task *t);
void task_resume_lock(struct task *t);
void task_wakeup(struct task *t);

/* Called by FPB when a task hits a breakpoint */
void task_hit_breakpoint(struct task *t);


/* Functions targeting the Current (Running) task
 * */
int task_in_syscall(void);
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
/* Validate userspace pointers passed in the syscalls. */
int task_ptr_valid(const void *ptr);
/* Slice off one unit from the task current run time */
int task_timeslice(void);
struct fnode *task_getcwd(void);
void task_chdir(struct fnode *f);

/* Naming is not nice, implications of renaming is refactoring */
void sleepy_task_wakeup(uint32_t now, void *arg);

/* Manage timer IDs */
int task_get_timer_id(void);
void task_set_timer_id(int id);

/* Semaphores */
int sem_wait(sem_t *s, struct timespec *t);
int sem_trywait(sem_t *s);
int sem_post(sem_t *s);
int sem_init(sem_t *s, int val);
int sem_destroy(sem_t *s);

/* Mutexen */
int mutex_lock(mutex_t *s);
int mutex_trylock(mutex_t *s);
int mutex_unlock(mutex_t *s);
mutex_t *mutex_init();
void mutex_destroy(mutex_t *s);

#define schedule()   *((uint32_t volatile *)0xE000ED04) = 0x10000000

/* Timers */
int Timer_on(unsigned int n);

/* Tasklets */
void tasklet_add(void (*exe)(void*), void *arg);
void check_tasklets(void);

/* Kthreads */
struct task *kthread_create(void (routine)(void *), void *arg);
void kthread_sleep_ms(uint32_t ms);
void task_yield(void);
void kthread_yield(void);
int kthread_cancel(struct task *t);

/* Modules */
struct module *MODS;
int register_module(struct module *m);
int unregister_module(struct module *m);
struct module *module_search(char *name);

/* System */
int sys_register_handler(uint32_t n, int (*_sys_c)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t));
int syscall(uint32_t syscall_nr, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
void syscalls_init(void);
int task_is_interrupted(void);

#define SYS_CALL_AGAIN_VAL (-1024)
//#define SYS_CALL_AGAIN (task_is_interrupted()?(-EINTR):(SYS_CALL_AGAIN_VAL))
#define SYS_CALL_AGAIN SYS_CALL_AGAIN_VAL

/* VFS */
void vfs_init(void);
int fno_fullpath(struct fnode *f, char *dst, int len);
struct fnode *fno_create_file(const char *path);
void fno_use(struct fnode *f);

#define FL_RDONLY 0x01
#define FL_WRONLY 0x02
#define FL_RDWR   (FL_RDONLY | FL_WRONLY)
#define FL_DIR    0x04
#define FL_INUSE  0x08
#define FL_TTY    0x10
#define FL_BLK    0x20

#define FL_EXEC   0x40
#define FL_LINK   0x80

#ifndef FD_CLOEXEC
    #define FD_CLOEXEC	1
#endif

#ifndef F_DUPFD
    #define F_DUPFD 0
#endif

#ifndef F_GETFD
    #define F_GETFD 1
#endif

#ifndef F_SETFD
    #define F_SETFD 2
#endif

#ifndef F_GETFL
    #define F_GETFL 3
#endif

#ifndef F_SETFL
    #define F_SETFL 4
#endif


struct fnode {
    struct module *owner;
    char *fname;
    char *linkname;
    uint32_t flags;
    struct fnode *parent;
    struct fnode *children;
    void *priv;
    uint32_t size;
    uint32_t off;
    int32_t usage_count;
    struct fnode *next;
};

#define FNO_MOD_PRIV(fno,mod) (((fno == NULL)?NULL:((mod != fno->owner)?NULL:(fno->priv))))
#define FNO_BLOCKING(f) ((f->flags & O_NONBLOCK) == 0)

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
int register_addr_family(struct module *m, uint16_t family);


#define FAMILY_UNIX 0x0001
#define FAMILY_INET 0x0002
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
        int (*truncate)(struct fnode *fno, unsigned int size);
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
        int (*getsockname)(int fd, struct sockaddr *addr, unsigned int *addrlen);
        int (*getpeername)(int fd, struct sockaddr *addr, unsigned int *addrlen);


        /* Terminal operations */
        void (*tty_attach)(struct fnode *fno, int pid);
        int (*tty_getsid)(struct fnode *fno);
        int (*tcsetattr)(int td, int opts, const struct termios *tp);
        int (*tcgetattr)(int td, struct termios *tp);

        /* Block device operations */
        int (*block_read)(struct fnode *fno, void *buf, uint32_t sector, int offset, int count);
        int (*block_write)(struct fnode *fno, const void *buf, uint32_t sector, int offset, int count);

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
#define F_MALLOC_OVERHEAD 24
uint32_t mem_stats_frag(int pool);
int fmalloc_owner(const void *ptr);
int fmalloc_chown(const void *ptr, uint16_t pid);

/* Helper defined by sysfs.c */
int ul_to_str(unsigned long n, char *s);
int sysfs_register(char *name, char *dir,
        int (*do_read)(struct sysfs_fnode *sfs, void *buf, int len),
        int (*do_write)(struct sysfs_fnode *sfs, const void *buf, int len) );
void sysfs_lock(void);
void sysfs_unlock(void);
void frosted_tcpip_wakeup(void);
void pico_lock_init(void);
void pico_lock(void);
void pico_unlock(void);

#endif /* BSP_INCLUDED_H */

