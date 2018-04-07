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

#include "frosted.h"
#include "sys/frosted.h"
#include "lowpower.h"
#include <time.h>
#include <unicore-mx/cm3/scb.h>

#ifndef CLOCK_MONOTONIC
# define CLOCK_MONOTONIC (4)
#endif

struct timeval_kernel
{
    /* Assuming newlib time_t is long */
    long tv_sec;
    long tv_usec;
};

int sys_suspend_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    if (arg1 > 0) {
        lowpower_sleep(0, arg1);
    }
    return 0;
}

int sys_standby_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    if (arg1 > 0) {
        lowpower_sleep(1, arg1);
    }
    return 0;
}

int sys_test_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    return (arg1 + (arg2 << 8));
}

int sys_thread_create_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    /* Deprecated. */
    return -1;
}

int sys_thread_join_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    /* Deprecated. */
    return -1;
}

int sys_execb_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    /* Deprecated. */
    return -1;
}


int sys_clock_gettime_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    if (arg2) {
        struct timeval_kernel *now = (struct timeval_kernel *)arg2;
        if ((clockid_t)arg1 == CLOCK_MONOTONIC) {
            now->tv_sec = jiffies / 1000;
            now->tv_usec = (jiffies % 1000) * 1000;
        } else if ((clockid_t)arg1 == CLOCK_REALTIME) {
            now->tv_sec = rt_offset + (jiffies / 1000);
            now->tv_usec = ((jiffies % 1000) * 1000) * 1000;
        }
    }
    return 0;
}

int sys_clock_settime_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    if ((clockid_t)arg1 == CLOCK_MONOTONIC) {
        return -EPERM;
    }
    if (arg2 && ((clockid_t)arg1 == CLOCK_REALTIME)) {
        struct timeval_kernel *now = (struct timeval_kernel *)arg2;
        unsigned int temp = now->tv_sec;
        temp = temp + (now->tv_usec / 1000 / 1000);
        rt_offset = temp - (jiffies / 1000);
    }
    return 0;
}

int sys_reboot_hdlr(void)
{
    scb_reset_system(); /* Never returns. */
}


struct utsname {
    char sysname[16];    /* Operating system name (e.g., "Frosted") */
    char nodename[16];   /* Name within network */
    char release[16];    /* Operating system release (e.g., "16.03") */
    char version[16];    /* Operating system version (e.g., "16") */
    char machine[16];    /* Hardware identifier */
    char domainname[16]; /* NIS or YP domain name */
};

const struct utsname uts_frosted = { "Frosted", "frosted", "16.03", "16", "arm", "local"};

int sys_uname_hdlr( uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    struct utsname *uts = (struct utsname *)arg1;
    if (!arg1)
        return -EFAULT;
    if (task_ptr_valid(uts))
        return -EACCES;
    memcpy(uts, &uts_frosted, sizeof(struct utsname));
    return 0;
}
