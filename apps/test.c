#include "frosted_api.h"
#include <string.h>
#include <stdio.h>
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
        sys_sleep(200);
    }
    (void)i;
}

#define LS_HDR "VFS Content:\n"

static void print_files(int ser, char *start, int level)
{
    char _fname[256];
    char *fname = _fname;
    struct dirent *ep;
    DIR *d;
    struct stat st;
    char size[10];
    char type;
    int i;

    d = sys_opendir(start);
    while(ep = sys_readdir(d)) {
        snprintf(fname, 256, "%s/%s", start, ep->d_name);    

        while (fname[0] == '/' && fname[1] == '/')
            fname++;

        sys_stat(fname, &st);
        for(i = 0; i < level; i++)
            sys_write(ser, "\t", 1);

        sys_write(ser, fname, strlen(fname));
        sys_write(ser, "\t", 1);
        if (S_ISDIR(st.st_mode))
            type = 'd';
        else
            type = 'f';
        snprintf(size, 10, "%c\t%d", type, st.st_size);
        sys_write(ser, size, strlen(size));
        sys_write(ser, "\n", 1);
        if (type == 'd') {
            print_files(ser, fname, level + 1);
        }
    }
    sys_closedir(d);
}


void task3(void *arg) {
    int ser;

    do {
        ser = sys_open("/dev/ttyS0", O_RDWR);
    } while (ser < 0);

    sys_write(ser, LS_HDR, strlen(LS_HDR));
    //print_files(ser, "/", 0);  /* Stat: work inprogress */
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
