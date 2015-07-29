#include "frosted.h"
#include "syscall_table.h"

/* TEMP for memset */
#include "string.h"

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


static void * _top_stack;
//#define EXT_RETURN ((struct nvic_stack_frame *)(_top_stack))->pc

//#define __inl __attribute__((always_inline))
#define __inl inline
#define __naked __attribute__((naked))


#define RUN_HANDLER (0xfffffff1u)
#define RUN_KERNEL  (0xfffffff9u)
#define RUN_USER    (0xfffffffdu)

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

static __inl void save_context(void)
{
    uint32_t tmp;
    asm volatile ("mrs %0, msp           \n" /* could be changed to msp or psp later */
         "stmdb %0!, {r4-r11}   \n"
         "msr msp, %0           \n" : "=r" (tmp));
}


static __inl void restore_context(void)
{
    uint32_t tmp;
    asm volatile ("mrs %0, msp           \n"
          "ldmfd %0!, {r4-r11}  \n"
          "msr msp, %0          \n" : "=r" (tmp));
}


static __inl void task_switch(void)
{
    int i, pid = _cur_task->pid;
    _cur_task->state = TASK_RUNNABLE;

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
    tasklist[pid].pid = pid;
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


/* C ABI cannot mess with the stack, we will */
void __naked  PendSV_Handler(void)
{
    /* disable interrupts */
    /// XXX TODO

    /* save current context on current stack */
    //save_context();
    //
    asm volatile (  "mrs r3, msp           \n" /* could be changed to msp or psp later */
                    "stmdb r3!, {r4-r11}   \n"
                    "msr msp, r3           \n" );

    /* save current SP to TCB */
    //_top_stack = msp_read();
    asm volatile ("mrs r3, msp" : "=r" (_top_stack));

    _cur_task->sp = _top_stack;

    asm volatile ("mov r7, lr");
    /* choose next task */
    task_switch();
    asm volatile ("mov lr, r7");

    //switch (pid) 
    //{
    //    case 0: /* KERNEL mode */
    //        //next_tcb = &tcb_user;
    //        _cur_task = &tasklist[1];
    //        break;

    //    case 1: /* USER mode, thread 1 */
    //        _cur_task = &tasklist[0];
    //        break;

    //    default:
    //        while(1);
    //        break;
    //}

    /* write new stack pointer */
    //msp_write(next_tcb->sp);
    asm volatile ("msr msp, r3" :: "r" (_cur_task->sp));

    /* restore context */
    //restore_context();
    asm volatile (  "mrs r3, msp          \n"
                    "ldmfd r3!, {r4-r11}  \n"
                    "msr msp, r3          \n" );
    
    /* enable interrupts */
    /// XXX TODO
    
    asm volatile (  "bx lr          \n" );
}

void __inl pendsv_enable(void)
{
   *((uint32_t volatile *)0xE000ED04) = 0x10000000; 
}

void kernel_task_init(void)
{
    /* task0 = kernel */
    task_create(0x0, 0x1337, 0); // kernel init value does not matter
    tasklist[0].sp = msp_read(); // but SP needs to be current SP
    _cur_task = &tasklist[0];
}

int scheduler_get_cur_pid(void)
{
    if (!_cur_task)
        return -1;
    return _cur_task->pid;
}
