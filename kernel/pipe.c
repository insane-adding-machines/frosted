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
 *
 */  
#include "frosted.h"
#include "cirbuf.h"
#include "string.h"
#include "sys/termios.h"
#include "poll.h"

#define PIPE_BUFSIZE 64

static struct module mod_pipe;
struct pipe_priv {
    int r;
    int w;
    int r_pid;
    int w_pid;
    int w_off;
    struct cirbuf *cb;
};

static struct fnode PIPE_ROOT = {
};

int sys_pipe2_hdlr(int paddr, int flags)
{
    int *pfd = (int*)paddr;
    struct fnode *new = fno_create(&mod_pipe, "", &PIPE_ROOT); 
    struct pipe_priv *pp;

    /* TODO: Implement flags */

    if (!new)
       return -ENOMEM; 

    pfd[0] = task_filedesc_add(new);
    pfd[1] = task_filedesc_add(new);
    if (pfd[0] < 0 || pfd[1] < 0)
        return -ENOMEM;
    
    pp = kalloc(sizeof (struct pipe_priv));
    if (!pp) {
        kfree(new);
        return -ENOMEM;
    }
    new->priv = pp;

    pp->r = pfd[0];
    pp->w = pfd[1];
    pp->r_pid = 0;
    pp->w_pid = 0;
    pp->w_off = 0;
    pp->cb = cirbuf_create(PIPE_BUFSIZE);
    if (!pp->cb) {
        kfree(pp);
        kfree(new);
        return -ENOMEM;
    }

    return 0;
}

static int pipe_poll(struct fnode *f, uint16_t events, uint16_t *revents)
{
    struct pipe_priv *pp;
    /* TODO: Check direction ! */
    if (f->owner != &mod_pipe)
        return -EINVAL;
    pp = (struct pipe_priv *)f->priv;
    if (!pp) {
        return -EINVAL;
    }

    if (pp->w < 0)
       *revents = POLLHUP; 

    if (pp->r < 0)
       *revents = POLLHUP; 

    if ((events & POLLIN) && (cirbuf_bytesinuse(pp->cb) > 0)) {
        *revents |= POLLIN;
        return 1;
    }
    if ((events & POLLOUT) && (cirbuf_bytesfree(pp->cb) > 0)) {
        *revents |= POLLOUT;
        return 1;
    }

    *revents = events;
    return 1;
}


static int pipe_close(struct fnode *f)
{
    struct pipe_priv *pp;
    uint16_t pid;
    pid = scheduler_get_cur_pid();

    if (f->owner != &mod_pipe)
        return -EINVAL;

    pp = (struct pipe_priv *)f->priv;
    if (!pp)
        return -EINVAL;

    /* TODO: implement a fork hook, so fnodes have usage count */
    if (pp->w_pid != pid) {
        pp->r = -1;
        if (pp->w_pid > 0)
            task_resume(pp->w_pid);
    }
    
    if (pp->r_pid != pid) {
        pp->w = -1;
        if (pp->r_pid > 0)
            task_resume(pp->r_pid);
    }

    if ((pp->w == -1) && (pp->r == -1)) {
        kfree(pp);
        fno_unlink(f);
    }
    return 0;
}

static int pipe_read(struct fnode *f, void *buf, unsigned int len)
{
    struct pipe_priv *pp;
    int out, len_available;
    uint8_t *ptr = buf;

    if (f->owner != &mod_pipe)
        return -EINVAL;

    pp = (struct pipe_priv *)f->priv;
    if (!pp)
        return -EINVAL;

    if (pp->w < 0)
        return -EPIPE;
    
    len_available =  cirbuf_bytesinuse(pp->cb);
    if (len_available <= 0) {
        pp->r_pid = scheduler_get_cur_pid();
        task_suspend();
        return SYS_CALL_AGAIN;
    }

    for(out = 0; out < len; out++) {
        /* read data */
        if (cirbuf_readbyte(pp->cb, ptr) != 0)
            break;
        ptr++;
    }
    pp->r_pid = 0;
    return out;
}

static int pipe_write(struct fnode *f, const void *buf, unsigned int len)
{
    struct pipe_priv *pp;
    int out, len_available;
    const uint8_t *ptr = buf;

    if (f->owner != &mod_pipe)
        return -EINVAL;

    pp = (struct pipe_priv *)f->priv;
    if (!pp)
        return -EINVAL;

    if (pp->r < 0)
        return -EPIPE;

    out = pp->w_off;
    
    len_available =  cirbuf_bytesfree(pp->cb);
    if (len_available > (len - out))
        len_available = (len - out);
    for(; out < len_available; out++) {
        /* write data */
        if (cirbuf_writebyte(pp->cb, *(ptr + out)) != 0)
            break;
    }

    if (out < len) {
        pp->w_pid = scheduler_get_cur_pid();
        pp->w_off = out;
        task_suspend();
        return SYS_CALL_AGAIN;
    }

    pp->w_off = 0;
    pp->w_pid = 0;
    return out;
}

void sys_pipe_init(void)
{
    mod_pipe.family = FAMILY_DEV;
    strcpy(mod_pipe.name,"pipe");
    mod_pipe.ops.poll = pipe_poll;
    mod_pipe.ops.close = pipe_close;
    mod_pipe.ops.read = pipe_read;
    mod_pipe.ops.write = pipe_write;


    register_module(&mod_pipe);
}


