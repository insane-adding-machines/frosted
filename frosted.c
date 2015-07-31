#include "frosted.h"

#define IDLE() while(1){do{}while(0);}


void frosted_init(void)
{
    SystemInit(); /* SystemInit() -> Board_SystemInit() */
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);
    Board_Init();

    syscalls_init();
    vfs_init();
    kernel_task_init();
    frosted_scheduler_on();
}


/* This comes up again ! */
void task2(void *arg)
{
    volatile int i = (int)arg;
    volatile int pid, ppid;
    int fd = sys_open("/dev/null", 0, 0);
    while(1) {
        i = jiffies;
        pid = sys_getpid();
        ppid = sys_getppid();
    }
    (void)i;
}

void task1(void *arg)
{
    volatile int i = (int)arg;
    volatile int pid;
    int fd;
    uint32_t *temp;
    /* c-lib and init test */
    temp = malloc(32);
    free(temp);

    /* open/close test */
    fd = sys_open("/dev/null", 0, 0);
    sys_close(fd);
    
    /* Thread create test */
    if (sys_thread_create(task2, (void *)42, 1) < 0)
        IDLE();

    while(1) {
        i = jiffies;
        //sys_sleep(500); /* Work in progress... */
        pid = sys_getpid();
    }
    (void)i;
}


extern void tcb_init(void);

void main(void) 
{
    volatile int ret = 0;

    frosted_init();

    if (task_create(task1, (void *)42, 2) < 0)
        IDLE();

    while(1) {
        /* This is the kernel main loop */   
    }
}

