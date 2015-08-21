#include "frosted_api.h"
#include "fresh.h"
#include <string.h>
#include <stdio.h>
#define IDLE() while(1){do{}while(0);}
#define GREETING "Welcome to frosted!\n"

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
    close(ser);

    addr = (void *)sys_malloc(20);
    ret = sys_mkdir("/mem/test");
    ret = sys_read(fdz, addr, 20);
    ret = sys_write(fdn, addr, 20);
    sys_close(fdn);
    sys_close(fdz);
    now = sys_gettimeofday(NULL);
    pid = sys_getpid();
    ppid = sys_getppid();
    sys_free(addr);

    while(1) {
        addr = (void *)sys_malloc(20);

        fdm = sys_open("/mem/test/test", O_RDWR|O_CREAT|O_TRUNC);
        if (fdm < 0)
            while(1);;
        ret = sys_write(fdm, "hello", 5);
        sys_close(fdm);

        fdm = sys_open("/mem/test/test", O_RDWR);
        ret = sys_read(fdm, addr, 20);

        sys_close(fdm);
        sys_unlink("/mem/test/test");

        sys_free(addr);
    }
    (void)i;
}

void idling(void *arg)
{
    int pid;
    while(1) {
        pid = sys_getpid();
        sys_sleep(100);
    }
}

static struct semaphore *sem = NULL;
void prod(void *arg)
{
    sem = sys_sem_init(0);
    while(1) {
        sys_sem_post(sem);
        sys_sleep(1000);
    }
}

void cons(void *arg)
{
    volatile int counter = 0;
    while(!sem)
        sys_sleep(200);
    while(1) {
        sys_sem_wait(sem);
        counter++;
    }
}

void init(void *arg)
{
    volatile int i = (int)arg;
    volatile int pid;
    int fd, sd;
    uint32_t *temp;
    /* c-lib and init test */
    temp = (uint32_t *)sys_malloc(32);
    sys_free(temp);

    /* open/close test */
    fd = sys_open("/dev/null", 0);
    sys_close(fd);
    
    /* socket/close test */
    sd = sys_socket(AF_UNIX, SOCK_DGRAM, 0);
    sys_close(sd);


    /* Thread create test */
    if (sys_thread_create(idling, (void *)42, 1) < 0)
        IDLE();
    if (sys_thread_create(fresh, (void *)42, 1) < 0)
        IDLE();
    if (sys_thread_create(prod, (void *)42, 1) < 0)
        IDLE();
    if (sys_thread_create(cons, (void *)42, 1) < 0)
        IDLE();

    while(1) {
        sys_sleep(500);
        pid = sys_getpid();
    }
    (void)i;
}
