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
    }
    (void)i;
}

#define LS_HDR " **** VFS Content: **** \n"
#define LS_TAIL " **** End of VFS Content. ****\n\n"

static void print_files(int ser, char *start, int level)
{
    char *fname;
    char *fname_start;
    struct dirent *ep;
    DIR *d;
    struct stat st;
    char type;
    int i;

    fname_start = sys_malloc(MAX_FILE);
    ep = sys_malloc(sizeof(struct dirent));
    if (!ep || !fname_start)
        while(1);;

    d = sys_opendir(start);
    while(sys_readdir(d, ep) == 0) {
        fname = fname_start;
        fname[0] = '\0';
        strncat(fname, start, MAX_FILE);
        strncat(fname, "/", MAX_FILE);
        strncat(fname, ep->d_name, MAX_FILE);

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

        sys_write(ser, &type, 1); 
        sys_write(ser, "\n", 1);
    }
    sys_closedir(d);
    sys_free(ep);
    sys_free(fname_start);
}

static const char str_welcome[]      = "Welcome to Frosted\r\n";
static const char str_unknowncmd[]   = "Unknown command. Try 'help'.\r\n";
static const char str_help[]         = "The only supported commands are 'help' and 'ls'.\r\n";
static const char str_prompt[]       = "[frosted]$> ";

void task3(void *arg) {
    int ser;

    do {
        ser = sys_open("/dev/ttyS0", O_RDWR);
    } while (ser < 0);

    sys_write(ser, str_welcome, strlen(str_welcome));

    while (2>1)
    {
        char input[100];
        int len;

        sys_write(ser, str_prompt, strlen(str_prompt));

        len = sys_read(ser, input, 100);
        sys_write(ser, "\n", 1);

        input[len] = '\0';
        if (!strncmp(input, "ls", 2))
        {
            sys_write(ser, LS_HDR, strlen(LS_HDR));
            print_files(ser, "/", 0);  /* Stat: work inprogress */
            print_files(ser, "/mem", 0);  /* Stat: work inprogress */
            print_files(ser, "/dev", 0);  /* Stat: work inprogress */
            sys_write(ser, LS_TAIL, strlen(LS_TAIL));
        } else if (!strncmp(input, "help", 4)) {
            sys_write(ser, str_help, strlen(str_help));
        } else {
            sys_write(ser, str_unknowncmd, strlen(str_unknowncmd));
        }
    }

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
    //if (sys_thread_create(task2, (void *)42, 1) < 0)
    //    IDLE();
    if (sys_thread_create(task3, (void *)42, 1) < 0)
        IDLE();

    while(1) {
        sys_sleep(500);
        pid = sys_getpid();
    }
    (void)i;
}
