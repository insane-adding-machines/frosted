#include "frosted.h"
#include "syscall_table.h"


/* Full kernel space separation */
#define RUN_HANDLER (0xfffffff1u)
#ifdef USERSPACE
#   define MSP "msp"
#   define PSP "psp"
#define RUN_KERNEL  (0xfffffff9u)
#define RUN_USER    (0xfffffffdu)

/* Separation is emulated */
#else
#   define MSP "msp"
#   define PSP "msp"
#define RUN_KERNEL  RUN_HANDLER
#define RUN_USER    RUN_HANDLER
#endif


/* XXX TEMP for memset */
#include "string.h"

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
#define TIMESLICE(x) ((BASE_TIMESLICE) + (x->prio << 2))
#define STACK_SIZE (996)

struct __attribute__((packed)) nvic_stack_frame {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
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
//#define EXT_RETURN ((struct nvic_stack_frame *)(_top_stack))->pc

//#define __inl __attribute__((always_inline))
#define __inl inline
#define __naked __attribute__((naked))



#define TASK_IDLE       0
#define TASK_RUNNABLE   1
#define TASK_RUNNING    2
#define TASK_SLEEPING   3
#define TASK_WAITING    4
#define TASK_OVER 0xFF


struct __attribute__((packed)) task {
    void (*start)(void *);
    void *arg;
    uint16_t state;
    uint16_t prio;
    int timeslice;
    uint16_t pid;
    uint16_t ppid;
    void *sp;
    uint32_t stack[STACK_SIZE / 4];
};

volatile struct task *_cur_task = NULL;

static struct task tasklist[MAX_TASKS] __attribute__((section(".task"), __used__)) = {}; 
static int pid_max = 0;
static __inl int in_kernel(void)
{
    return (_cur_task == tasklist);
}

static __inl void * psp_read(void)
{
    void * ret=NULL;
    asm volatile ("mrs %0, psp" : "=r" (ret));
    return ret;
}

static __inl void psp_write(void *in)
{
    asm volatile ("msr psp, %0" :: "r" (in));
}

static __inl void * msp_read(void)
{
    void * ret=NULL;
    asm volatile ("mrs %0, msp" : "=r" (ret));
    return ret;
}

static __inl void msp_write(void *in)
{
    asm volatile ("msr msp, %0" :: "r" (in));
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


int task_timeslice(void) 
{
    return (--_cur_task->timeslice);
}

int task_create(void (*init)(void *), void *arg, unsigned int prio)
{
    struct nvic_stack_frame *nvic_frame;
    struct extra_stack_frame *extra_frame;
    int pid = pid_max;
    uint8_t *sp;
    if (pid_max >= MAX_TASKS)
        return -1;
    irq_off();
    tasklist[pid].pid = pid;
    tasklist[pid].ppid = scheduler_get_cur_pid();
    tasklist[pid].prio = prio;
    tasklist[pid].start = init;
    tasklist[pid].arg = arg;
    tasklist[pid].timeslice = TIMESLICE((&tasklist[pid]));
    tasklist[pid].state = TASK_RUNNABLE;
    sp = (((uint8_t *)(&tasklist[pid].stack)) + STACK_SIZE - NVIC_FRAME_SIZE);

    /* Stack frame is at the end of the stack space */
    nvic_frame = (struct nvic_stack_frame *) sp;
    memset(nvic_frame, 0, NVIC_FRAME_SIZE);
    nvic_frame->r0 = (uint32_t) arg;
    nvic_frame->pc = (uint32_t) init;
    nvic_frame->lr = (uint32_t) RUN_HANDLER;
    nvic_frame->psr = 0x21000000ul;
    sp -= EXTRA_FRAME_SIZE;
    extra_frame = (struct extra_stack_frame *)sp;
    //extra_frame->lr = RUN_USER;
    tasklist[pid].sp = (uint32_t *)sp;
    tasklist[pid].state = TASK_RUNNABLE;
    pid_max++;
    irq_on();

    return 0;
} 

static __naked void save_kernel_context(void)
{
    asm volatile ("mrs r0, "MSP"           ");
    asm volatile ("stmdb r0!, {r4-r11}   ");
    asm volatile ("msr "MSP", r0           ");
    asm volatile ("bx lr                 ");
}

static __naked void save_task_context(void)
{
    asm volatile ("mrs r0, "PSP"           ");
    asm volatile ("stmdb r0!, {r4-r11}   ");
    asm volatile ("msr "PSP", r0           ");
    asm volatile ("bx lr                 ");
}


static uint32_t runnable = RUN_HANDLER;

static __naked void restore_kernel_context(void)
{
    asm volatile ("mrs r0, "MSP"          ");
    asm volatile ("ldmfd r0!, {r4-r11}  ");
    asm volatile ("msr "MSP", r0          ");
    asm volatile ("bx lr                 ");
}

static __naked void restore_task_context(void)
{
    asm volatile ("mrs r0, "PSP"          ");
    asm volatile ("ldmfd r0!, {r4-r11}  ");
    asm volatile ("msr "PSP", r0          ");
    asm volatile ("bx lr                 ");
}


/* C ABI cannot mess with the stack, we will */
void __naked  PendSV_Handler(void)
{

    /* save current context on current stack */
    if (in_kernel()) {
        save_kernel_context();
        asm volatile ("mrs %0, "MSP"" : "=r" (_top_stack));
    } else {
        save_task_context();
        asm volatile ("mrs %0, "PSP"" : "=r" (_top_stack));
    }

    /* save current SP to TCB */
    //_top_stack = msp_read();

    _cur_task->sp = _top_stack;
    _cur_task->state = TASK_RUNNABLE;

    /* choose next task */
    task_switch();

    /* write new stack pointer and restore context */
    if (in_kernel()) {
        asm volatile ("msr "MSP", %0" :: "r" (_cur_task->sp));
        restore_kernel_context();
        runnable = RUN_KERNEL;
    } else {
        asm volatile ("msr "PSP", %0" :: "r" (_cur_task->sp));
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

int sys_sleep_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    uint16_t pid = scheduler_get_cur_pid();

    if (arg1 < 0)
        return -1;

    if (pid > 0) {
        _cur_task->state = TASK_SLEEPING;
        _cur_task->timeslice = jiffies + arg1;
    }
    return 0;
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

    call = sys_syscall_handlers[n];
    retval = call(arg1, arg2, arg3, *a4, *a5);

    if ((_cur_task->state == TASK_SLEEPING) || (_cur_task->state == TASK_WAITING))
        task_switch();
    else {
        asm volatile ( "mov %0, r0" : "=r" 
                (*((uint32_t *)(_cur_task->sp + EXTRA_FRAME_SIZE))) );
    }

    /* write new stack pointer and restore context */
    if (in_kernel()) {
        asm volatile ("msr "MSP", %0" :: "r" (_cur_task->sp));
        restore_kernel_context();
        runnable = RUN_KERNEL;
    } else {
        asm volatile ("msr "PSP", %0" :: "r" (_cur_task->sp));
        restore_task_context();
        runnable = RUN_USER;
    }

    /* Set return value selected by the restore procedure */ 
    asm volatile ("mov lr, %0" :: "r" (runnable));

    /* return (function is naked) */ 
    asm volatile ( "bx lr");
}
