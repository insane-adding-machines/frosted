#include "frosted.h"
#include <string.h>

#define IDLE() while(1){do{}while(0);}
#define GREETING "Welcome to frosted!\n"


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
    void *addr;
    int fdn = sys_open("/dev/null", O_RDONLY);
    int fdz = sys_open("/dev/zero", O_WRONLY);
    int fdm;
    int ser;
    volatile int test_retval = sys_test(0x10,0x20,0x30,0x40,0x50);
    volatile uint32_t now; 
    volatile int ret;

    ser = sys_open("/dev/ttyS0", O_RDWR);
    sys_write(ser, GREETING, strlen(GREETING));

    ret = sys_mkdir("/mem/test");

    while(1) {
        addr = (void *)sys_malloc(20);
        ret = sys_read(fdz, addr, 20);
        ret = sys_write(fdn, addr, 20);
        sys_close(fdn);
        sys_close(fdz);
        now = sys_gettimeofday(NULL);
        pid = sys_getpid();
        ppid = sys_getppid();

        fdm = sys_open("/mem/test/test", O_RDWR|O_CREAT|O_TRUNC);
        ret = sys_write(fdm, "hello", 5);
        sys_close(fdm);

        fdm = sys_open("/mem/test/test", O_RDWR);
        ret = sys_read(fdm, addr, 20);

        sys_unlink("/mem/test");
        sys_close(fdm);

        sys_free(addr);
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
    fd = sys_open("/dev/null", 0);
    sys_close(fd);
    
    /* Thread create test */
    if (sys_thread_create(task2, (void *)42, 1) < 0)
        IDLE();

    while(1) {
        i = jiffies;
        sys_sleep(500);
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

