#include "frosted.h"
#include "syscall_table.h"

#define MAX_TASKS 16
#define PRIO 2
#define BASE_TIMESLICE (20)
#define TIMESLICE(x) ((BASE_TIMESLICE) + (PRIO << 2))
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


static uint32_t * _top_stack;
//#define EXT_RETURN ((struct nvic_stack_frame *)(_top_stack))->pc


#define __inl __attribute__((always_inline))
//#define __inl inline


#define RUN_HNDLER 0xfffffff1u
#define RUN_KERNEL 0xfffffff9u
#define RUN_USER   0xfffffffdu

#define TASK_IDLE       0
#define TASK_RUNNABLE   1
#define TASK_RUNNING    2
#define TASK_SLEEPING   3
#define TASK_WAITING    4
#define TASK_OVER 0xFF

struct task {
    void (*start)(void *);
    void *arg;
    int state;
    int timeslice;
    int pid;
    void *sp;
    uint32_t stack[STACK_SIZE / 4];
};

static volatile struct task *_cur_task = NULL;
static volatile uint32_t tmp;

static struct task tasklist[MAX_TASKS] __attribute__((section(".task"), __used__)) = {}; 
static int pid_max = 0;

static __inl void * psp_read(void)
{
    void * ret=NULL;
    asm ("mrs %0, psp" : "=r" (ret));
    return ret;
}

static __inl void * msp_read(void)
{
    void * ret=NULL;
    asm ("mrs %0, msp" : "=r" (ret));
    return ret;
}

static __inl void psp_write(void *in)
{
    asm ("msr psp, %0" :: "r" (in));
}

static __inl void save_context(void)
{
    asm ("mrs %0, psp\n"
            "stmdb %0!, {r4-r11}\n"
            "msr psp, %0\n" : "=r" (tmp));
}


static __inl void load_context(void)
{
    asm ("mrs %0, psp\n"
            "ldmfd %0!, {r4-r11}\n"
            "msr psp, %0\n" : "=r" (tmp));
}


static int i, pid;
static __inl void task_switch(void)
{
    if (!_cur_task && (pid_max == 0))
        return;
    if (_cur_task) {
        pid = _cur_task->pid;
        *_top_stack = RUN_USER;
    } else {
        pid = 0;
        *_top_stack = RUN_KERNEL;
    }

    for (i = pid + 1 ;; i++) {
        if (i >= pid_max)
            i = 0;
        if (tasklist[i].state == TASK_RUNNABLE) {
            _cur_task = &tasklist[i];
            break;
        }
    }
    _cur_task->timeslice = TIMESLICE(_cur_task);
    _cur_task->state = TASK_RUNNING;
}



void task_end(void)
{
    /* TODO: schedule again. */
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
    nvic_frame->lr = (uint32_t) task_end;
    nvic_frame->psr = 0x21000000ul;
    sp -= EXTRA_FRAME_SIZE;
    extra_frame = (struct extra_stack_frame *)sp;
    //extra_frame->lr = RUN_USER;
    tasklist[pid].sp = (uint32_t *)sp;
    tasklist[pid].state = TASK_RUNNABLE;
    pid_max++;
    irq_on();
} 




//void __attribute__((naked)) PendSV_Handler(void)
void PendSV_Handler(void)
{
    _top_stack = msp_read();
    if (_cur_task) {
        asm volatile ("mrs %0, psp\n"
                "stmdb %0!, {r4-r11}\n"
                "msr psp, %0\n" :: "r" (_cur_task->sp)
                );

    }

    task_switch();
    /* 
    if (_cur_task)
        _cur_task->timeslice -= _clock_interval;
    if (!_cur_task || (_cur_task->timeslice <= 0)) {
        task_switch();
    }
    */

    asm volatile ("mrs %0, psp\n"
            "ldmfd %0!, {r4-r11}\n"
            "msr psp, %0\n" : "=r" (_cur_task->sp)
            );

    //asm ("bx lr\n");

}

void pendsv_enable(void)
{
   *((uint32_t volatile *)0xE000ED04) = 0x10000000; 
}

int __attribute__((signal)) SVC_Handler(uint32_t syscall_nr, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    struct nvic_stack_frame *nvic_frame;
    struct extra_stack_frame *extra_frame;
    volatile void *tmp;
    //irq_off();
    switch(syscall_nr) {
        case SYS_START:
            _top_stack = msp_read();
            
            _cur_task = &tasklist[0];
            _cur_task->state = TASK_RUNNING;
            *_top_stack = RUN_USER;
            _syscall_retval = 0;
            frosted_scheduler_on();
            asm volatile ( "ldmfd %0!, {r4-r11}\n" "msr psp, %0\n" : "+r" (_cur_task->sp));
            break;
        case SYS_STOP:
            frosted_scheduler_off();
            _syscall_retval = 0;
            break;
        case SYS_SETCLOCK:
            _syscall_retval = SysTick_interval(arg1);
            break;
        case SYS_SLEEP:
            Timer_on(arg1);
            _syscall_retval = 0;
            break;
        case SYS_THREAD_CREATE:
            _syscall_retval = task_create((void (*)(void *))arg1, (void *)arg2, arg3);
            break;
        default: 
            _syscall_retval = -1;
    }
    //irq_on();
    //asm ("bx lr\n");
}

