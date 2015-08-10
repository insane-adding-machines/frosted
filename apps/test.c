#include "frosted_api.h"
#include <string.h>
#define IDLE() while(1){do{}while(0);}
#define GREETING "Welcome to frosted!\n"

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
    close(ser);

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

        sys_close(fdm);
        sys_unlink("/mem/test/test");

        sys_free(addr);
    }
    (void)i;
}

void task3(void *arg) {
    volatile uint32_t now; 
    int ser;
    struct dirent *ep;
    DIR *d;
    do {
        now = sys_gettimeofday(NULL);
    } while (now < 1000) ;

    do {
        ser = sys_open("/dev/ttyS0", O_RDWR);
    } while (ser < 0);

    d = sys_opendir("/");
    while (ep = sys_readdir(d)) {
        sys_write(ser, ep->d_name, strlen(ep->d_name));
        sys_write(ser, "\n", 1);
    }
    sys_closedir(d);
    close(ser);

}



void init(void *arg)
{
    volatile int i = (int)arg;
    volatile int pid;
    int fd;
    uint32_t *temp;
    /* c-lib and init test */
    temp = (uint32_t *)sys_malloc(32);
    sys_free(temp);

    /* open/close test */
    fd = sys_open("/dev/null", 0);
    sys_close(fd);
    
    /* Thread create test */
    if (sys_thread_create(task2, (void *)42, 1) < 0)
        IDLE();
    if (sys_thread_create(task3, (void *)42, 1) < 0)
        IDLE();

    while(1) {
        sys_sleep(500);
        pid = sys_getpid();
    }
    (void)i;
}
