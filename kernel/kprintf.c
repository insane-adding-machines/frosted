/*
 *  frosted - printf() library function
 *
 Based on Georges Menie's printf -
 Copyright 2001, 2002 Georges Menie (www.menie.org)
 stdarg version contributed by Christian Ettinger
 Originally released under LGPL2+.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License version 2, as
 published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdint.h>
#include <string.h>
#include <sys/vfs.h>
#include "frosted.h"
#include "errno.h"
#include "cirbuf.h"
#include "poll.h"
#include "device.h"
#include <stdarg.h>

#ifdef CONFIG_KLOG
struct dev_klog {
    struct fnode *fno;
    struct cirbuf *buf;
	int used;
    int pid;
};

static struct dev_klog klog;


static int klog_open(const char *path, int flags)
{
    if (klog.used)
        return -EBUSY;
    klog.used++;
    return task_filedesc_add(fno_search("/dev/klog"));
}

static int klog_close(struct fnode *f)
{
    (void)f;
    klog.used = 0;
    return 0;
}

static int klog_read(struct fnode *fno, void *buf, unsigned int len)
{
    int ret;
    if (len == 0)
        return len;
    if (!buf)
        return -EINVAL;

    ret = cirbuf_readbytes(klog.buf, buf, len);
    if (ret <= 0) {
        klog.pid = scheduler_get_cur_pid();
        task_suspend();
        return SYS_CALL_AGAIN;
    }
    return ret;
}

static int klog_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    if (events != POLLIN)
        return 0;
    if (cirbuf_bytesinuse(klog.buf) > 0) {
        *revents = POLLIN;
        return 1;
    }
    return 0;
}

static struct module mod_klog = {
    .family = FAMILY_DEV,
    .name = "klog",
    .ops.open = klog_open,
    .ops.read = klog_read,
    .ops.poll = klog_poll,
    .ops.close = klog_close
};



static void printchar(char **str, int c)
{
    if (str) {
        **str = c;
        ++(*str);
    }
    else {
        if (cirbuf_bytesfree(klog.buf)) {
            cirbuf_writebyte(klog.buf, c);
            if (klog.pid > 0) 
                task_resume(klog.pid);
        }
    }
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad)
{
    int pc = 0, padchar = ' ';

    if (width > 0) {
        int len = 0;
        const char *ptr;
        for (ptr = string; *ptr; ++ptr) ++len;
        if (len >= width) width = 0;
        else width -= len;
        if (pad & PAD_ZERO) padchar = '0';
    }
    if (!(pad & PAD_RIGHT)) {
        for ( ; width > 0; --width) {
            printchar (out, padchar);
            ++pc;
        }
    }
    for ( ; *string ; ++string) {
        printchar (out, *string);
        ++pc;
    }
    for ( ; width > 0; --width) {
        printchar (out, padchar);
        ++pc;
    }

    return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
    char print_buf[PRINT_BUF_LEN];
    char *s;
    int t, neg = 0, pc = 0;
    unsigned int u = i;

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints (out, print_buf, width, pad);
    }

    if (sg && b == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN-1;
    *s = '\0';

    while (u) {
        t = u % b;
        if( t >= 10 )
            t += letbase - '0' - 10;
        *--s = t + '0';
        u /= b;
    }

    if (neg) {
        if( width && (pad & PAD_ZERO) ) {
            printchar (out, '-');
            ++pc;
            --width;
        }
        else {
            *--s = '-';
        }
    }

    return pc + prints (out, s, width, pad);
}

static int print(char **out, const char *format, va_list args )
{
    int width, pad;
    int pc = 0;
    char scr[2];

    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = pad = 0;
            if (*format == '\0') break;
            if (*format == '%') goto out;
            if (*format == '-') {
                ++format;
                pad = PAD_RIGHT;
            }

            while ((*format == 'h') || (*format == 'l'))
                ++format;

            while (*format == '0') {
                ++format;
                pad |= PAD_ZERO;
            }
            for ( ; *format >= '0' && *format <= '9'; ++format) {
                width *= 10;
                width += *format - '0';
            }
            if( *format == 's' ) {
                register char *s = (char *)va_arg( args, int );
                pc += prints (out, s?s:"(null)", width, pad);
                continue;
            }
            if( *format == 'd' ) {
                pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
                continue;
            }
            if( *format == 'x' || *format == 'p') {
                pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
                continue;
            }
            if( *format == 'X' ) {
                pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
                continue;
            }
            if( *format == 'u' ) {
                pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
                continue;
            }
            if( *format == 'c' ) {
                scr[0] = (char)va_arg( args, int );
                scr[1] = '\0';
                pc += prints(out, scr, width, pad);
                continue;
            }
        }
        else {
out:
            printchar (out, *format);
            ++pc;
        }
    }
    if (out) **out = '\0';
    va_end( args );
    return pc;
}

static mutex_t *klog_lock;

int kprintf(const char *format, ...)
{
    va_list args;
    int ret;
    if (cirbuf_bytesfree(klog.buf)) {
        if (mutex_trylock(klog_lock) < 0)
            return 0;
        va_start(args, format);
        ret = print(0, format, args);
        mutex_unlock(klog_lock);
        return ret;
    }
    return 0;
}

int ksprintf(char *out, const char *format, ...)
{
    va_list args;
    va_start( args, format );
    return print( &out, format, args );
}

int klog_init(void)
{
    klog.fno = fno_create_rdonly(&mod_klog, "klog", fno_search("/dev"));
    if (klog.fno == NULL) {
        return -1;
    }
    klog.buf = cirbuf_create(CONFIG_KLOG_SIZE);
	klog.used = 0;
    klog.pid = 0;
    klog_lock = mutex_init();
    return 0;
}
#endif
