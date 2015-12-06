/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the Free Software Foundation.
 *      
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted_api.h"
#include "fresh.h"
#include "syscalls.h"
#include "ioctl.h"
#include <string.h>
#include <stdio.h>
#define IDLE() while(1){do{}while(0);}
#define GREETING "Welcome to frosted!\n"

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
    int led[4];
    int i, j;

#ifdef PYBOARD
# define LED0 "/dev/gpio_1_13"
# define LED1 "/dev/gpio_1_14"
# define LED2 "/dev/gpio_1_15"
# define LED3 "/dev/gpio_1_4"
#elif defined (STM32F4)
# define LED0 "/dev/gpio_3_12"
# define LED1 "/dev/gpio_3_13"
# define LED2 "/dev/gpio_3_14"
# define LED3 "/dev/gpio_3_15"
#elif defined (LPC17XX)
#if 0
/*LPCXpresso 1769 */
# define LED0 "/dev/gpio_0_22"
# define LED1 "/dev/null"
# define LED2 "/dev/null"
# define LED3 "/dev/null"
#else
/* mbed 1768 */
# define LED0 "/dev/gpio_1_18"
# define LED1 "/dev/gpio_1_20"
# define LED2 "/dev/gpio_1_21"
# define LED3 "/dev/gpio_1_23"
#endif
#else
# define LED0 "/dev/null"
# define LED1 "/dev/null"
# define LED2 "/dev/null"
# define LED3 "/dev/null"
#endif

    led[0] = open(LED0, O_RDWR, 0);
    led[1] = open(LED1, O_RDWR, 0);
    led[2] = open(LED2, O_RDWR, 0);
    led[3] = open(LED3, O_RDWR, 0);

    if (led[0] >= 0) {
        while(1) {
            for (i = 0; i < 9; i++) {
                if (i < 4) {
                    write(led[i], "0", 1);
                } else {
                    char val = (1 - (i % 2)) + '0';
                    for(j = 0; j < 4; j++)
                        write(led[j], &val, 1);
                }
                sleep(200);
            }
        }
    } else {
        while(1) { sleep(1000); } /* GPIO unavailable, just sleep. */
    }
}

static sem_t *sem = NULL;
static frosted_mutex_t *mut = NULL;
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

void main(void *arg)
{
    volatile int i = (int)arg;
    volatile int pid;
    int fd, sd;
    uint32_t *temp;
    int testval = 42;
    /* c-lib and init test */
    temp = (uint32_t *)malloc(32);
    free(temp);

    /* open/close test */
    fd = open("/dev/null", 0, 0);
    printf("/dev/null %s a tty.\r\n",isatty(fd)?"is":"is not");
    close(fd);
    
    /* socket/close test */
    sd = socket(AF_UNIX, SOCK_DGRAM, 0);
    close(sd);

    /* Thread create test */
    if (thread_create(idling, &testval, 1) < 0)
        IDLE();
    if (thread_create(fresh, &testval, 1) < 0)
        IDLE();

    while(1) {
        sleep(500);
        pid = getpid();
    }
    (void)i;
}

void _start(void)
{
    //_init();
    main(NULL);
}

