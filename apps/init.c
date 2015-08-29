#include "frosted_api.h"
#include "fresh.h"
#include <string.h>
#include <stdio.h>
#define IDLE() while(1){do{}while(0);}
#define GREETING "Welcome to frosted!\n"

 int (** const __syscall__)( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t ) = (int (**const)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)) 0xF0; 

void task2(void *arg)
{
    volatile int i = (int)arg;
    volatile int pid, ppid;
    void *addr;
    int fdn = open("/dev/null", O_RDONLY, 0);
    int fdz = open("/dev/zero", O_WRONLY, 0);
    int fdm;
    int ser;
    volatile uint32_t now; 
    volatile int ret;
    ser = open("/dev/ttyS0", O_RDWR, 0);
    write(ser, GREETING, strlen(GREETING));
    close(ser);

    addr = (void *)malloc(20);
    ret = mkdir("/mem/test");
    ret = read(fdz, addr, 20);
    ret = write(fdn, addr, 20);
    close(fdn);
    close(fdz);
    now = gettimeofday(NULL);
    pid = getpid();
    ppid = getppid();
    free(addr);

    while(1) {
        addr = (void *)malloc(20);

        fdm = open("/mem/test/test", O_RDWR|O_CREAT|O_TRUNC, 0);
        if (fdm < 0)
            while(1);;
        ret = write(fdm, "hello", 5);
        close(fdm);

        fdm = open("/mem/test/test", O_RDWR, 0);
        ret = read(fdm, addr, 20);

        close(fdm);
        unlink("/mem/test/test");

        free(addr);
    }
    (void)i;
}

void idling(void *arg)
{
    int pid;
    while(1) {
        pid = getpid();
        sleep(100);
    }
}

static sem_t *sem = NULL;
static mutex_t *mut = NULL;
void prod(void *arg)
{
    sem = sem_init(0);
    mut = mutex_init();
    while(1) {
        sem_post(sem);
        sleep(1000);
    }
}

void cons(void *arg)
{
    volatile int counter = 0;
    while(!sem)
        sleep(200);
    while(1) {
        sem_wait(sem);
        //mutex_lock(mut);
        counter++;
        //mutex_unlock(mut);
    }
}

void init(void *arg)
{
    volatile int i = (int)arg;
    volatile int pid;
    int fd, sd;
    uint32_t *temp;
    /* c-lib and init test */
    temp = (uint32_t *)malloc(32);
    free(temp);

    /* open/close test */
    fd = open("/dev/null", 0, 0);
    close(fd);
    
    /* socket/close test */
    sd = socket(AF_UNIX, SOCK_DGRAM, 0);
    close(sd);


    /* Thread create test */
    if (thread_create(idling, (void *)42, 1) < 0)
        IDLE();
    if (thread_create(fresh, (void *)42, 1) < 0)
        IDLE();
    if (thread_create(prod, (void *)42, 1) < 0)
        IDLE();
    if (thread_create(cons, (void *)42, 1) < 0)
        IDLE();

    while(1) {
        sleep(500);
        pid = getpid();
    }
    (void)i;
}

//*****************************************************************************
//
// The entry point for the application.
//
//*****************************************************************************
extern int main(void);
extern unsigned int __StackTop; /* provided by linker script */
 
 
//*****************************************************************************
//
// The following are constructs created by the linker, indicating where the
// the "data" and "bss" segments reside in memory.  The initializers for the
// "data" segment resides immediately following the "text" segment.
//
//*****************************************************************************
extern unsigned long apps_etext;
extern unsigned long apps_data;
extern unsigned long apps_edata;
extern unsigned long apps_bss;
extern unsigned long apps_ebss;
 
//*****************************************************************************
//
// This is the code that gets called when the processor first starts execution
// following a reset event.  Only the absolutely necessary set is performed,
// after which the application supplied entry() routine is called. 
//
//*****************************************************************************
void INIT _init(void)
{
    unsigned long *pulSrc, *pulDest;
    unsigned char * bssDest;
 
    //
    // Copy the data segment initializers from flash to SRAM.
    //
    pulSrc = &apps_etext;
    pulDest = &apps_data; 

    while(pulDest < &apps_edata)
    {
        *pulDest++ = *pulSrc++;
    }

    //
    // Zero-init the BSS section
    //
    bssDest = (unsigned char *)&apps_bss;

    while(bssDest < (unsigned char *)&apps_ebss)
    {
        *bssDest++ = 0u;
    }
 
    //
    // Call the application's entry point.
    //
    init(NULL);
}
