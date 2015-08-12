#ifndef FROSTED_INCLUDED_H
#define FROSTED_INCLUDED_H

#define KERNEL
#include "frosted_api.h"

//#define DEBUG

#include <stdlib.h>
#include <stdint.h>
#include "system.h"

/* Types */
struct task;
struct fnode;

/* generics */
volatile unsigned int jiffies;
volatile int _syscall_retval;
void frosted_init(void);
void klog_set_write(int (*wr)(int, const void *, unsigned int));

/* klog */
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



/* Systick & co. */
int _clock_interval;
void SysTick_Hook(void);
int SysTick_Interval(unsigned long interval);
void SysTick_on(void);
void SysTick_off(void);

/* scheduler */
uint16_t scheduler_get_cur_pid(void);
uint16_t scheduler_get_cur_ppid(void);
int task_timeslice(void);
int task_filedesc_add(struct fnode *f);
struct fnode *task_filedesc_get(int fd);
int task_filedesc_del(int fd);

#define schedule()   *((uint32_t volatile *)0xE000ED04) = 0x10000000 


/* System */
int sys_register_handler(uint32_t n, int (*_sys_c)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t));
int syscall(uint32_t syscall_nr, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
void syscalls_init(void);

/* VFS */
void vfs_init(void);

#define FL_RDONLY 0x01
#define FL_WRONLY 0x02
#define FL_RDWR   (FL_RDONLY | FL_WRONLY)
#define FL_DIR    0x04
#define FL_INUSE  0x08

struct fnode {
    struct module *owner;
    char *fname;
    uint32_t flags;
    struct fnode *parent;
    struct fnode *children;
    void *priv;
    uint32_t size;
    uint32_t off;
    struct fnode *next;
};



struct fnode *fno_create(struct module *owner, const char *name, struct fnode *parent);
struct fnode *fno_mkdir(struct module *owner, const char *name, struct fnode *parent);
void fno_unlink(struct fnode *fno);

struct fnode *fno_search(const char *path);
struct fnode *fno_get(int fd);


/* Modules (for files/sockets) */


#define FAMILY_UNIX 0x0000
#define FAMILY_INET 0x0001
#define FAMILY_DEV  0x0DEF
#define FAMILY_FILE 0xFFFF


struct module {
    uint16_t family;

    struct module_operations {
        /* Common module operations */
        int (*read)(int fd, void *buf, unsigned int len);
        int (*write)(int fd, const void *buf, unsigned int len);
        int (*poll)(int fd, uint16_t events);
        int (*close)(int fd);

        /* Files only (NULL == socket) */
        int (*open)(const char *path, int flags);
        int (*seek)(int fd, int offset, int whence);
        int (*creat)(struct fnode *fno);
        int (*unlink)(struct fnode *fno);

        /* Sockets only (NULL == file) */
        int (*socket)(int domain, int type, int protocol);
        int (*recvfrom)(int fd, void *buf, unsigned int len, void *addr, int addrlen);
        int (*sendto)(int fd, const void *buf, unsigned int len, void *addr, int addrlen);
        int (*bind)(int fd, void *addr, int addrlen);
        int (*accept)(int fd, void *addr, int *addrlen);
        int (*connect)(int fd, const void *addr, int addrlen);
        int (*listen)(int fd, int backlog);
    } ops;
    struct module *next;
};



#ifdef DEBUG
static void irq_off(void)
{
}

static void irq_on(void)
{
}

#else

/* Inline kernel utils */
static inline void irq_off(void)
{
    asm volatile ("cpsid i\n");
}

static inline void irq_on(void)
{
    asm volatile ("cpsie i\n");
}
#endif

void task_run(void);
void kernel_task_init(void);



#define kalloc malloc
#define kcalloc calloc
#define krealloc realloc
#define kfree  free

#endif /* BSP_INCLUDED_H */

