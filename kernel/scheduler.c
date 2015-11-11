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
#define TIMESLICE(x) ((BASE_TIMESLICE) + ((x)->prio << 2))
#define STACK_SIZE (988)

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

struct __attribute__((packed)) task {
    void (*start)(void *);
    void *arg;
    uint16_t state;
    uint16_t prio;
    uint16_t timeslice;
    uint16_t pid;
    uint16_t ppid;
    uint16_t n_files;
    struct fnode *cwd;
    struct fnode **filedesc;
    void *sp;
    uint32_t stack[STACK_SIZE / 4];
};

volatile struct task *_cur_task = NULL;

static struct task tasklist[MAX_TASKS] __attribute__((section(".task"), __used__)) = {}; 
static int pid_max = 0;

/* Handling of file descriptors */
int task_filedesc_add(struct fnode *f)
{
    int i;
    void *re;
    volatile struct task *t = _cur_task;
    if (!t)
        return -1;
    for (i = 0; i < t->n_files; i++) {
        if (t->filedesc[i] == NULL) {
            t->filedesc[i] = f;
            return i;
        }
    }
    t->n_files++;
    re = (void *)krealloc(t->filedesc, t->n_files * sizeof(struct fnode *));
    if (!re)
        return -1;
    t->filedesc = re;
    t->filedesc[t->n_files - 1] = f;
    return t->n_files - 1;
}

struct fnode *task_filedesc_get(int fd)
{
    volatile struct task *t = _cur_task;
    if (fd < 0)
        return NULL;
    if (!t)
        return NULL;
    if (!t->filedesc || (( t->n_files - 1) < fd))
        return NULL;
    return t->filedesc[fd];
}

int task_filedesc_del(int fd)
{
    volatile struct task *t = _cur_task;
    if (!t)
        return -1;
    t->filedesc[fd] = NULL;
}

int sys_dup_hdlr(int fd)
{
    volatile struct task *t = _cur_task;
    struct fnode *f = task_filedesc_get(fd);
    int newfd = -1;
    if (!f)
        return -1;
    return task_filedesc_add(f);
}

int sys_dup2_hdlr(int fd, int newfd)
{
    volatile struct task *t = _cur_task;
    struct fnode *f = task_filedesc_get(fd);
    if (newfd < 0)
        return -1;
    if (newfd == fd)
        return -1;
    if (!f)
        return -1;
    if (newfd >= t->n_files)
        return -1;
    if (t->filedesc[newfd] != NULL)
        return -1;
    t->filedesc[newfd] = f;
    return newfd;
}

struct fnode *task_getcwd(void)
{
    return _cur_task->cwd;
}

void task_chdir(struct fnode *f)
{
    _cur_task->cwd = f;
}

static __inl int in_kernel(void)
{
    return (_cur_task == tasklist);
}

static __inl void * msp_read(void)
{
    void * ret=NULL;
    asm volatile ("mrs %0, msp" : "=r" (ret));
    return ret;
}

static __inl void task_switch(void)
{
    int i, pid = _cur_task->pid;

    for (i = pid + 1 ;; i++) {
        if (i >= pid_max)
            i = 0;
        if ((i == 0) || (tasklist[i].state == TASK_RUNNABLE)) {
            _cur_task = &tasklist[i];
            break;
        }
        if ((tasklist[i].state == TASK_SLEEPING)) {
            if (tasklist[i].timeslice <= jiffies) {
                _cur_task = &tasklist[i];
                break;
            }
        }
    }
    _cur_task->timeslice = TIMESLICE(_cur_task);
    _cur_task->state = TASK_RUNNING;
}

int scheduler_ntasks(void)
{
    return pid_max;
}

int scheduler_task_state(int pid)
{
    return tasklist[pid].state;
}

unsigned scheduler_stack_used(int pid)
{
    return STACK_SIZE - ((char *)tasklist[pid].sp - (char *)tasklist[pid].stack);
}

uint16_t scheduler_get_cur_pid(void)
{
    if (!_cur_task)
        return 0;
    return _cur_task->pid;
}

uint16_t scheduler_get_cur_ppid(void)
{
    if (!_cur_task)
        return 0;
    return _cur_task->ppid;
}

int task_running(void)
{
    return (_cur_task->state == TASK_RUNNING);
}

int task_timeslice(void) 
{
    return (--_cur_task->timeslice);
}

void task_end(void)
{
    _cur_task->state = TASK_OVER;
    while(1);
}

int task_create(void (*init)(void *), void *arg, unsigned int prio)
{
    struct nvic_stack_frame *nvic_frame;
    struct extra_stack_frame *extra_frame;
    int pid = pid_max;
    uint8_t *sp;
    if (pid_max >= MAX_TASKS)
        return -1;
    //irq_setmask();
    irq_off();
    tasklist[pid].pid = pid;
    tasklist[pid].ppid = scheduler_get_cur_pid();
    tasklist[pid].prio = prio;
    tasklist[pid].start = init;
    tasklist[pid].arg = arg;
    tasklist[pid].filedesc = NULL;
    tasklist[pid].n_files = 0;
    tasklist[pid].timeslice = TIMESLICE((&tasklist[pid]));
    tasklist[pid].state = TASK_RUNNABLE;
    tasklist[pid].cwd = fno_search("/");
    sp = (((uint8_t *)(&tasklist[pid].stack)) + STACK_SIZE - NVIC_FRAME_SIZE);

    /* Stack frame is at the end of the stack space */
    nvic_frame = (struct nvic_stack_frame *) sp;
    memset(nvic_frame, 0, NVIC_FRAME_SIZE);
    nvic_frame->r0 = (uint32_t) arg;
    nvic_frame->pc = (uint32_t) init;
    nvic_frame->lr = (uint32_t) task_end;
    nvic_frame->psr = 0x01000000u;
    sp -= EXTRA_FRAME_SIZE;
    extra_frame = (struct extra_stack_frame *)sp;
    //extra_frame->lr = RUN_USER;
    tasklist[pid].sp = (uint32_t *)sp;
    tasklist[pid].state = TASK_RUNNABLE;
    pid_max++;
    //irq_clearmask();
    irq_on();

    return 0;
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
void __naked  PendSV_Handler(void)
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

    _cur_task->sp = _top_stack;
    if (_cur_task->state == TASK_RUNNING)
        _cur_task->state = TASK_RUNNABLE;

    /* choose next task */
    task_switch();
    
    if (((int)(_cur_task->sp) - (int)(&_cur_task->stack)) < STACK_THRESHOLD) {
        klog(LOG_WARNING, "Process %d is running out of stack space!\n", _cur_task->pid);
    }

    /* write new stack pointer and restore context */
    if (in_kernel()) {
        asm volatile ("msr "MSP", %0" :: "r" (_cur_task->sp));
        asm volatile ("isb");
        asm volatile ("msr CONTROL, %0" :: "r" (0x00));
        asm volatile ("isb");
        restore_kernel_context();
        runnable = RUN_KERNEL;
    } else {
        asm volatile ("msr "PSP", %0" :: "r" (_cur_task->sp));
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
    task_create(0x0, (void *)0x1337, 0); // kernel init value does not matter
    tasklist[0].sp = msp_read(); // but SP needs to be current SP
    _cur_task = &tasklist[0];
}


void task_suspend(void)
{
    _cur_task->state = TASK_WAITING;
    _cur_task->timeslice = 0;
    schedule();
}


void task_resume(int pid)
{
    if (tasklist[pid].state == TASK_WAITING) {
        tasklist[pid].state = TASK_RUNNABLE;
        tasklist[pid].timeslice = TIMESLICE(&tasklist[pid]);
    }
}

void task_terminate(int pid)
{
    tasklist[pid].state = TASK_OVER;
    tasklist[pid].timeslice = 0;
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
        return -1;

    if (pid > 0) {
        ktimer_add(arg1, sleepy_task_wakeup, (void *)_cur_task->pid);
        if (timeout < jiffies) 
            return 0;

        task_suspend();
        return SYS_CALL_AGAIN;
    }
    return 0;
}

int sys_kill_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    if (arg1 > pid_max)
        return -1;
    /* For now, signal param is ignored. */
    task_terminate(arg1);
    return 0;
}

int sys_exit_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg)
{
    task_terminate(_cur_task->pid);
}

static uint32_t *a4 = NULL;
static uint32_t *a5 = NULL;
int __attribute__((naked)) SVC_Handler(uint32_t n, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    /* save current context on current stack */
    save_task_context();
    asm volatile ("mrs %0, "PSP"" : "=r" (_top_stack));


    /* save current SP to TCB */
    _cur_task->sp = _top_stack;
    a4 = (uint32_t *)((uint8_t *)_cur_task->sp + (EXTRA_FRAME_SIZE + NVIC_FRAME_SIZE));
    a5 = (uint32_t *)((uint8_t *)_cur_task->sp + (EXTRA_FRAME_SIZE + NVIC_FRAME_SIZE + 4));

    /* Execute syscall */
    int retval;
    int (*call)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) = NULL;

    if (n >= _SYSCALLS_NR)
        return -1;
    if (sys_syscall_handlers[n] == NULL)
        return -1;

    //irq_setmask();
    irq_off();
    call = sys_syscall_handlers[n];
    retval = call(arg1, arg2, arg3, *a4, *a5);
    //irq_clearmask();
    irq_on();

    if ((_cur_task->state == TASK_SLEEPING) || (_cur_task->state == TASK_WAITING))
        task_switch();
    else {
        asm volatile ( "mov %0, r0" : "=r" 
                (*((uint32_t *)(_cur_task->sp + EXTRA_FRAME_SIZE))) );
    }

    /* write new stack pointer and restore context */
    if (in_kernel()) {
        asm volatile ("msr "MSP", %0" :: "r" (_cur_task->sp));
        asm volatile ("isb");
        asm volatile ("msr CONTROL, %0" :: "r" (0x00));
        asm volatile ("isb");
        restore_kernel_context();
        runnable = RUN_KERNEL;
    } else {
        asm volatile ("msr "PSP", %0" :: "r" (_cur_task->sp));
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
