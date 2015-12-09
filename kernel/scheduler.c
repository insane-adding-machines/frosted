/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the Free Software Foundation.
 *      
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted.h"
#include "syscall_table.h"
#include "string.h" /* flibc string.h */


/* Full kernel space separation */
#define RUN_HANDLER (0xfffffff1u)
#define MSP "msp"
#define PSP "psp"
#define RUN_KERNEL  (0xfffffff9u)
#define RUN_USER    (0xfffffffdu)


#define STACK_THRESHOLD 64


/* Array of syscalls */
static void *sys_syscall_handlers[_SYSCALLS_NR] = {

};

int sys_register_handler(uint32_t n, int(*_sys_c)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5))
{
    if (n >= _SYSCALLS_NR)
        return -1; /* Attempting to register non-existing syscall */

    if (sys_syscall_handlers[n] != NULL)
        return -1; /* Syscall already registered */

    sys_syscall_handlers[n] = _sys_c;
    return 0;
} 


#define MAX_TASKS 16
#define BASE_TIMESLICE (20)
#define TIMESLICE(x) ((BASE_TIMESLICE) + ((x)->tb.prio << 2))
#define STACK_SIZE (1024)
#define INIT_STACK_SIZE (256)

struct __attribute__((packed)) nvic_stack_frame {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
#   if (__CORTEX_M == 4) /* CORTEX-M4 saves FPU frame as well */
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t s12;
    uint32_t s13;
    uint32_t s14;
    uint32_t s15;
    uint32_t fpscr;
    uint32_t dummy;
#   endif
};
struct __attribute__((packed)) extra_stack_frame {
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
};

#define NVIC_FRAME_SIZE ((sizeof(struct nvic_stack_frame)))
#define EXTRA_FRAME_SIZE ((sizeof(struct extra_stack_frame)))


static void * _top_stack;
#define __inl inline
#define __naked __attribute__((naked))


#define TASK_FLAG_VFORK 0x01

struct filedesc {
    struct fnode *fno;
    uint32_t mask;
};


struct __attribute__((packed)) task_block {
    void (*start)(void *);
    void *arg;
    uint8_t state;
    uint8_t flags;
    uint16_t prio;
    uint16_t timeslice;
    uint16_t pid;
    uint16_t ppid;
    uint16_t n_files;
    struct fnode *cwd;
    struct filedesc *filedesc;
    void *sp;
    struct task *next;
};

struct __attribute__((packed)) task {
    struct task_block tb;
    uint32_t stack[STACK_SIZE / 4];
};

static struct task struct_task_init;
static struct task_block struct_task_block_kernel;
static struct task *const kernel = (struct task *)(&struct_task_block_kernel);

static int number_of_tasks = 0;

static void tasklist_add(struct task **list, struct task *el)
{
    el->tb.next = *list;
    *list = el;
}

static int tasklist_del(struct task **list, uint16_t pid)
{
    struct task *t = *list;
    struct task *p = NULL;

    while (t) {
        if (t->tb.pid == pid) {
            if (p == NULL)
                *list = t->tb.next;
            else
                p->tb.next = t->tb.next;
            return 0;
        }
        p = t;
        t = t->tb.next;
    }
    return -1;
}

static struct task *tasklist_get(struct task **list, uint16_t pid)
{
    struct task *t = *list;
    while (t) {
        if (t->tb.pid == pid)
            return t;
        t = t->tb.next;
    }
    return NULL;

}

static struct task *tasks_running = NULL;
static struct task *tasks_idling = NULL;

static void idling_to_running(struct task *t)
{
    if (tasklist_del(&tasks_idling, t->tb.pid) == 0)
        tasklist_add(&tasks_running, t);
}

static void running_to_idling(struct task *t)
{
    if (tasklist_del(&tasks_running, t->tb.pid) == 0)
        tasklist_add(&tasks_idling, t);
}

static void task_destroy(struct task *t)
{
    tasklist_del(&tasks_idling, t->tb.pid);
    task_space_free(t);
    number_of_tasks--;
}

volatile struct task *_cur_task = NULL;


static int next_pid(void)
{
    static unsigned int next_available = 0;
    uint16_t ret = (uint16_t)((next_available) & 0xFFFF);
    next_available++;
    if (next_available > 0xFFFF) {
        next_available = 2;
    }
    while(tasklist_get(&tasks_idling, next_available) || tasklist_get(&tasks_running, next_available))
        next_available++;
    return ret;
}


/* Handling of file descriptors */
static int task_filedesc_add_to_task(volatile struct task *t, struct fnode *f)
{
    int i;
    void *re;
    if (!t || !f)
        return -EINVAL;
    for (i = 0; i < t->tb.n_files; i++) {
        if (t->tb.filedesc[i].fno == NULL) {
            t->tb.filedesc[i].fno = f;
            f->usage++;
            return i;
        }
    }
    t->tb.n_files++;
    re = (void *)krealloc(t->tb.filedesc, t->tb.n_files * sizeof(struct filedesc));
    if (!re)
        return -1;
    t->tb.filedesc = re;
    memset(&(t->tb.filedesc[t->tb.n_files - 1]), 0, sizeof(struct filedesc));
    t->tb.filedesc[t->tb.n_files - 1].fno = f;
    f->usage++;
    return t->tb.n_files - 1;
}

int task_filedesc_add(struct fnode *f)
{
    return task_filedesc_add_to_task(_cur_task, f);
}

int task_fd_setmask(int fd, uint32_t mask)
{
    struct fnode *fno = _cur_task->tb.filedesc[fd].fno;
    if (!fno)
        return -EINVAL;

    if (mask & O_RDONLY) {
        if ((fno->flags & FL_RDONLY)== 0)
            return -EPERM;
    }
    if (mask & O_WRONLY) {
        if ((fno->flags & FL_WRONLY)== 0)
            return -EPERM;
    }

    _cur_task->tb.filedesc[fd].mask = mask;
    return 0;
}

uint32_t task_fd_getmask(int fd)
{
    if (_cur_task->tb.filedesc[fd].fno)
        return _cur_task->tb.filedesc[fd].mask;
    return 0;
}

struct fnode *task_filedesc_get(int fd)
{
    struct task *t = _cur_task;
    if (fd < 0)
        return NULL;
    if (fd >= t->tb.n_files)
        return NULL;
    if (!t)
        return NULL;
    if (!t->tb.filedesc || (( t->tb.n_files - 1) < fd))
        return NULL;
    if (t->tb.filedesc[fd].fno == NULL)
        return NULL;
    return t->tb.filedesc[fd].fno;
}

int task_fd_readable(int fd)
{
    if (!task_filedesc_get(fd) || ((_cur_task->tb.filedesc[fd].mask & O_RDONLY) == 0))
        return 0;
    return 1;
}

int task_fd_writable(int fd)
{
    if (!task_filedesc_get(fd) || ((_cur_task->tb.filedesc[fd].mask & O_WRONLY) == 0))
        return 0;
    return 1;
}

int task_filedesc_del(int fd)
{
    struct task *t = _cur_task;
    if (!t)
        return -EINVAL;
    if (!t->tb.filedesc[fd].fno)
        return -ENOENT;
    t->tb.filedesc[fd].fno->usage--;
    t->tb.filedesc[fd].fno = NULL;
}

int sys_dup_hdlr(int fd)
{
    struct task *t = _cur_task;
    struct fnode *f = task_filedesc_get(fd);
    int newfd = -1;
    if (!f)
        return -1;
    return task_filedesc_add(f);
}

int sys_dup2_hdlr(int fd, int newfd)
{
    struct task *t = _cur_task;
    struct fnode *f = task_filedesc_get(fd);
    if (newfd < 0)
        return -1;
    if (newfd == fd)
        return -1;
    if (!f)
        return -1;
    if (newfd >= t->tb.n_files)
        return -1;
    if (t->tb.filedesc[newfd].fno != NULL)
        return -1;
    t->tb.filedesc[newfd].fno = f;
    return newfd;
}

struct fnode *task_getcwd(void)
{
    return _cur_task->tb.cwd;
}

void task_chdir(struct fnode *f)
{
    _cur_task->tb.cwd = f;
}

static __inl int in_kernel(void)
{
    return (_cur_task->tb.pid == 0);
}

static __inl void * msp_read(void)
{
    void * ret=NULL;
    asm volatile ("mrs %0, msp" : "=r" (ret));
    return ret;
}

static __inl void task_switch(void)
{
    int i, pid = _cur_task->tb.pid;
    struct task *t = _cur_task;

    if (((t->tb.state != TASK_RUNNING) && (t->tb.state != TASK_RUNNABLE)) || (t->tb.next == NULL))
        t = tasks_running;
    else
        t = t->tb.next;
    t->tb.timeslice = TIMESLICE(t);
    t->tb.state = TASK_RUNNING;
    _cur_task = t;
}

int scheduler_ntasks(void)
{
    return number_of_tasks;
}

int scheduler_task_state(int pid)
{
    struct task *t = tasklist_get(&tasks_running, pid);
    if (!t) 
        t = tasklist_get(&tasks_idling, pid);
    if (t)
        return t->tb.state;
    else return TASK_OVER;
}

unsigned scheduler_stack_used(int pid)
{
    struct task *t = tasklist_get(&tasks_running, pid);
    if (!t) 
        t = tasklist_get(&tasks_idling, pid);
    if (t)
        return STACK_SIZE - ((char *)t->tb.sp - (char *)t->stack);
    else return 0;
}

uint16_t scheduler_get_cur_pid(void)
{
    if (!_cur_task)
        return 0;
    return _cur_task->tb.pid;
}

uint16_t scheduler_get_cur_ppid(void)
{
    if (!_cur_task)
        return 0;
    return _cur_task->tb.ppid;
}

int task_running(void)
{
    return (_cur_task->tb.state == TASK_RUNNING);
}

int task_timeslice(void) 
{
    return (--_cur_task->tb.timeslice);
}

void task_end(void)
{
    running_to_idling(_cur_task);
    _cur_task->tb.state = TASK_ZOMBIE;
    while(1) {
        if (_cur_task->tb.ppid > 0)
            task_resume(_cur_task->tb.ppid);
        task_suspend();
    }
}

static void task_create_real(volatile struct task *new, void (*init)(void *), void *arg, unsigned int prio, uint32_t r9val)
{
    struct nvic_stack_frame *nvic_frame;
    struct extra_stack_frame *extra_frame;
    uint8_t *sp;

    new->tb.start = init;
    new->tb.arg = arg;
    new->tb.timeslice = TIMESLICE(new);
    new->tb.state = TASK_RUNNABLE;
    new->tb.cwd = fno_search("/");

    
    /* stack memory */
    sp = (((uint8_t *)(&new->stack)) + STACK_SIZE - NVIC_FRAME_SIZE);

    /* Stack frame is at the end of the stack space */
    nvic_frame = (struct nvic_stack_frame *) sp;
    memset(nvic_frame, 0, NVIC_FRAME_SIZE);
    nvic_frame->r0 = (uint32_t) arg;
    nvic_frame->pc = (uint32_t) init;
    nvic_frame->lr = (uint32_t) task_end;
    nvic_frame->psr = 0x01000000u;
    sp -= EXTRA_FRAME_SIZE;
    extra_frame = (struct extra_stack_frame *)sp;
    extra_frame->r9 = r9val;
    new->tb.sp = (uint32_t *)sp;
} 

int task_create_GOT(void (*init)(void *), void *arg, unsigned int prio, uint32_t got_loc)
{
    struct task *new;
    int i;

    irq_off();
    if (number_of_tasks == 0) {
        new = &struct_task_init;
    } else {
        new = task_space_alloc(sizeof(struct task));
    }
    if (!new) {
        return -ENOMEM;
    }
    new->tb.pid = next_pid();
    new->tb.ppid = scheduler_get_cur_pid();
    new->tb.prio = prio;
    new->tb.filedesc = NULL;
    new->tb.n_files = 0;
    new->tb.flags = 0;

    /* Inherit cwd, file descriptors from parent */
    if (new->tb.ppid > 1) { /* Start from parent #2 */
        new->tb.cwd = task_getcwd();
        for (i = 0; i < _cur_task->tb.n_files; i++) {
            task_filedesc_add_to_task(new, _cur_task->tb.filedesc[i].fno);
        }
    } 

    new->tb.next = NULL;
    tasklist_add(&tasks_running, new);

    number_of_tasks++;
    task_create_real(new, init, arg, prio, got_loc);
    new->tb.state = TASK_RUNNABLE;
    irq_on();
    return new->tb.pid;
}


int task_create(void (*init)(void *), void *arg, unsigned int prio)
{
    struct task *new;
    int i;

    irq_off();
    if (number_of_tasks == 0) {
        new = &struct_task_init;
    } else {
        new = task_space_alloc(sizeof(struct task));
    }
    if (!new) {
        return -ENOMEM;
    }
    new->tb.pid = next_pid();
    new->tb.ppid = scheduler_get_cur_pid();
    new->tb.prio = prio;
    new->tb.filedesc = NULL;
    new->tb.n_files = 0;
    new->tb.flags = 0;

    /* Inherit cwd, file descriptors from parent */
    if (new->tb.ppid > 1) { /* Start from parent #2 */
        new->tb.cwd = task_getcwd();
        for (i = 0; i < _cur_task->tb.n_files; i++) {
            task_filedesc_add_to_task(new, _cur_task->tb.filedesc[i].fno);
        }
    } 

    new->tb.next = NULL;
    tasklist_add(&tasks_running, new);

    number_of_tasks++;
    task_create_real(new, init, arg, prio, 0);
    new->tb.state = TASK_RUNNABLE;
    irq_on();
    return new->tb.pid;
}

int scheduler_exec(void (*init)(void *), void *arg)
{
    volatile struct task *t = _cur_task;
    task_create_real(t, init, arg, t->tb.prio, 0);
    return 0;
}

int scheduler_vfork(void)
{
    struct task *new;
    int i;

    irq_off();
    new = task_space_alloc(sizeof(struct task_block));
    if (!new) {
        return -ENOMEM;
    }
    new->tb.pid = next_pid();
    new->tb.ppid = scheduler_get_cur_pid();
    new->tb.prio = _cur_task->tb.prio;
    new->tb.filedesc = NULL;
    new->tb.n_files = 0;
    new->tb.flags = TASK_FLAG_VFORK;

    /* Inherit cwd, file descriptors from parent */
    if (new->tb.ppid > 1) { /* Start from parent #2 */
        new->tb.cwd = task_getcwd();
        for (i = 0; i < _cur_task->tb.n_files; i++) {
            task_filedesc_add_to_task(new, _cur_task->tb.filedesc[i].fno);
        }
    } 

    new->tb.next = NULL;
    tasklist_add(&tasks_running, new);
    number_of_tasks++;
    new->tb.sp = _cur_task->tb.sp;

    new->tb.state = TASK_RUNNABLE;
    irq_on();
    return new->tb.pid;
}

static __naked void save_kernel_context(void)
{
    asm volatile ("mrs r0, "MSP"           ");
    asm volatile ("stmdb r0!, {r4-r11}   ");
    asm volatile ("msr "MSP", r0           ");
    asm volatile ("isb");
    asm volatile ("bx lr                 ");
}

static __naked void save_task_context(void)
{
    asm volatile ("mrs r0, "PSP"           ");
    asm volatile ("stmdb r0!, {r4-r11}   ");
    asm volatile ("msr "PSP", r0           ");
    asm volatile ("isb");
    asm volatile ("bx lr                 ");
}


static uint32_t runnable = RUN_HANDLER;

static __naked void restore_kernel_context(void)
{
    asm volatile ("mrs r0, "MSP"          ");
    asm volatile ("ldmfd r0!, {r4-r11}  ");
    asm volatile ("msr "MSP", r0          ");
    asm volatile ("isb");
    asm volatile ("bx lr                 ");
}

static __naked void restore_task_context(void)
{
    asm volatile ("mrs r0, "PSP"          ");
    asm volatile ("ldmfd r0!, {r4-r11}  ");
    asm volatile ("msr "PSP", r0          ");
    asm volatile ("isb");
    asm volatile ("bx lr                 ");
}


/* C ABI cannot mess with the stack, we will */
void __naked  pend_sv_handler(void)
{
    /* save current context on current stack */
    if (in_kernel()) {
        save_kernel_context();
        asm volatile ("mrs %0, "MSP"" : "=r" (_top_stack));
        asm volatile ("isb");
    } else {
        save_task_context();
        asm volatile ("mrs %0, "PSP"" : "=r" (_top_stack));
        asm volatile ("isb");
    }

    asm volatile ("isb");


    /* save current SP to TCB */
    //_top_stack = msp_read();

    _cur_task->tb.sp = _top_stack;
    if (_cur_task->tb.state == TASK_RUNNING)
        _cur_task->tb.state = TASK_RUNNABLE;

    /* choose next task */
    task_switch();
    
    if (((int)(_cur_task->tb.sp) - (int)(&_cur_task->stack)) < STACK_THRESHOLD) {
        klog(LOG_WARNING, "Process %d is running out of stack space!\n", _cur_task->tb.pid);
    }

    /* write new stack pointer and restore context */
    if (in_kernel()) {
        asm volatile ("msr "MSP", %0" :: "r" (_cur_task->tb.sp));
        asm volatile ("isb");
        asm volatile ("msr CONTROL, %0" :: "r" (0x00));
        asm volatile ("isb");
        restore_kernel_context();
        runnable = RUN_KERNEL;
    } else {
        asm volatile ("msr "PSP", %0" :: "r" (_cur_task->tb.sp));
        asm volatile ("isb");
        asm volatile ("msr CONTROL, %0" :: "r" (0x01));
        asm volatile ("isb");
        restore_task_context();
        runnable = RUN_USER;
    }

    /* Set return value selected by the restore procedure */ 
    asm volatile ("mov lr, %0" :: "r" (runnable));

    /* return (function is naked) */ 
    asm volatile ("bx lr          \n" );
}

void __inl pendsv_enable(void)
{
   *((uint32_t volatile *)0xE000ED04) = 0x10000000; 
}

void kernel_task_init(void)
{
    /* task0 = kernel */
    irq_off();
    kernel->tb.sp = msp_read(); // SP needs to be current SP
    kernel->tb.pid = next_pid();
    kernel->tb.ppid = scheduler_get_cur_pid();
    kernel->tb.prio = 0;
    kernel->tb.start = NULL;
    kernel->tb.arg = NULL;
    kernel->tb.filedesc = NULL;
    kernel->tb.n_files = 0;
    kernel->tb.timeslice = TIMESLICE(kernel);
    kernel->tb.state = TASK_RUNNABLE;
    kernel->tb.cwd = fno_search("/");
    kernel->tb.state = TASK_RUNNABLE;
    kernel->tb.next = NULL;
    tasklist_add(&tasks_running, kernel);
    irq_on();

    /* Set kernel as current task */
    _cur_task = kernel;
}


void task_suspend(void)
{
    running_to_idling(_cur_task);
    if (_cur_task->tb.state == TASK_RUNNABLE || _cur_task->tb.state == TASK_RUNNING) {
        _cur_task->tb.state = TASK_WAITING;
        _cur_task->tb.timeslice = 0;
    }
    schedule();
}


void task_resume(int pid)
{
    struct task *t = tasklist_get(&tasks_idling, pid);
    if ((t) && t->tb.state == TASK_WAITING) {
        idling_to_running(t);
        t->tb.state = TASK_RUNNABLE;
    }
}

void task_terminate(int pid)
{
    struct task *t = tasklist_get(&tasks_running, pid);
    if (!t) 
        t = tasklist_get(&tasks_idling, pid);
    else
        running_to_idling(t);

    if (t) {
        t->tb.state = TASK_ZOMBIE;
        t->tb.timeslice = 0;
        if (t->tb.ppid > 0)
            task_resume(t->tb.ppid);
    }
}

static void sleepy_task_wakeup(uint32_t now, void *arg)
{
    int pid = (int)arg;
    task_resume(pid);
}

int sys_sleep_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    uint16_t pid = scheduler_get_cur_pid();
    uint32_t timeout = jiffies + arg1;

    if (arg1 < 0)
        return -EINVAL;

    if (pid > 0) {
        ktimer_add(arg1, sleepy_task_wakeup, (void *)_cur_task->tb.pid);
        if (timeout < jiffies) 
            return 0;

        task_suspend();
    }
    return 0;
}

int sys_thread_join_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    int to_arg = (int)arg2;
    uint32_t timeout;
    struct task *t = NULL; 

    if (arg1 <= 1)
        return -EINVAL;

    t = tasklist_get(&tasks_running, arg1);
    if (!t)
        t = tasklist_get(&tasks_idling, arg1);


    if (!t || _cur_task->tb.pid == arg1 || t->tb.ppid != _cur_task->tb.pid)
        return -EINVAL;


    if (t->tb.state == TASK_ZOMBIE) {
        t->tb.state = TASK_OVER;
        task_destroy(t);
        return 0; /* TODO: get task return value from stacked NVIC frame -> r0 */
    }

    if (to_arg == 0)
        return -EINVAL;

    if (to_arg > 0)  {
        timeout = jiffies + to_arg;
        ktimer_add(to_arg, sleepy_task_wakeup, (void *)_cur_task->tb.pid);
        if (timeout < jiffies)
            return -ETIMEDOUT;
    }
    task_suspend();
    return SYS_CALL_AGAIN;
}

int sys_kill_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    /* For now, signal param is ignored. */
    task_terminate(arg1);
    return 0;
}

int sys_exit_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg)
{
    task_terminate(_cur_task->tb.pid);
}

static uint32_t *a4 = NULL;
static uint32_t *a5 = NULL;
int __attribute__((naked)) sv_call_handler(uint32_t n, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    /* save current context on current stack */
    save_task_context();
    asm volatile ("mrs %0, "PSP"" : "=r" (_top_stack));


    /* save current SP to TCB */
    _cur_task->tb.sp = _top_stack;
    a4 = (uint32_t *)((uint8_t *)_cur_task->tb.sp + (EXTRA_FRAME_SIZE + NVIC_FRAME_SIZE));
    a5 = (uint32_t *)((uint8_t *)_cur_task->tb.sp + (EXTRA_FRAME_SIZE + NVIC_FRAME_SIZE + 4));

    /* Execute syscall */
    volatile int retval;
    int (*call)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) = NULL;

    if (n >= _SYSCALLS_NR)
        return -1;
    if (sys_syscall_handlers[n] == NULL)
        return -1;

    irq_off();
    call = sys_syscall_handlers[n];
    retval = call(arg1, arg2, arg3, *a4, *a5);
    asm volatile ( "mov %0, r0" : "=r" 
            (*((uint32_t *)(_cur_task->tb.sp + EXTRA_FRAME_SIZE))) );
    irq_on();

    if (_cur_task->tb.state == TASK_WAITING) {
        task_switch();
    }

    /* write new stack pointer and restore context */
    if (in_kernel()) {
        asm volatile ("msr "MSP", %0" :: "r" (_cur_task->tb.sp));
        asm volatile ("isb");
        asm volatile ("msr CONTROL, %0" :: "r" (0x00));
        asm volatile ("isb");
        restore_kernel_context();
        runnable = RUN_KERNEL;
    } else {
        asm volatile ("msr "PSP", %0" :: "r" (_cur_task->tb.sp));
        asm volatile ("isb");
        asm volatile ("msr CONTROL, %0" :: "r" (0x01));
        asm volatile ("isb");
        restore_task_context();
        runnable = RUN_USER;
    }

    /* Set return value selected by the restore procedure */ 
    asm volatile ("mov lr, %0" :: "r" (runnable));

    /* return (function is naked) */ 
    asm volatile ( "bx lr");
}
