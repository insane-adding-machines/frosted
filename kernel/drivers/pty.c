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
 *      Authors: Daniele Lacamera
 *
 */
 
#include "frosted.h"
#include "device.h"
#include "poll.h"
#include "cirbuf.h"
#include <stdint.h>

#define PTS_BUFSIZE (256)


static struct devptmx {
    struct device *dev;
    uint16_t pty_idx;
    struct fnode PTMX_ROOT;
} PTMX;



struct devpty {
    struct fnode *fno;
    struct devpts *slave;
    struct task *task;
    uint16_t idx;
    uint16_t creator_pid;
    int sid;
};

struct devpts {
    struct device *dev;
    struct task *task;
    struct devpty *master;
    struct cirbuf *miso, *mosi;
    int sid;
};

int ptmx_open(const char *path, int flags);
int pty_open(const char *path, int flags);
static int pty_read(struct fnode *fno, void *buf, unsigned int len);
static int pty_poll(struct fnode *fno, uint16_t events, uint16_t *revents);
static int pty_write(struct fnode *fno, const void *buf, unsigned int len);
static void pty_tty_attach(struct fnode *fno, int pid);
static int pty_close(struct fnode *fno);
static int pts_read(struct fnode *fno, void *buf, unsigned int len);
static int pts_poll(struct fnode *fno, uint16_t events, uint16_t *revents);
static int pts_write(struct fnode *fno, const void *buf, unsigned int len);
static void pts_tty_attach(struct fnode *fno, int pid);
static int pts_close(struct fnode *fno);

static struct module mod_ptmx = {
    .family = FAMILY_DEV,
    .name = "ptmx",
    .ops.open = ptmx_open,
};

static struct module mod_devpty = {
    .family = FAMILY_DEV,
    .name = "pty",
    .ops.open = pty_open,
    .ops.read = pty_read,
    .ops.poll = pty_poll,
    .ops.write = pty_write,
    .ops.tty_attach = pty_tty_attach,
    .ops.close = pty_close

};

static struct module mod_devpts = {
    .family = FAMILY_DEV,
    .name = "pts",
    .ops.open = device_open,
    .ops.read = pts_read,
    .ops.poll = pts_poll,
    .ops.write = pts_write,
    .ops.tty_attach = pts_tty_attach
};

static int pts_create(void)
{
    int idx = PTMX.pty_idx++;
    struct devpts *pts;
    struct devpty *pty = kalloc(sizeof(struct devpty));
    char name[3];
    if (!pty)
        return -ENOMEM;
    pts = kalloc(sizeof(struct devpts));
    if (!pts) {
        kfree(pty);
        return -ENOMEM;
    }

    if (idx < 10) {
        name[0] = '0' + idx;
        name[1] = '\0';
    } else {
        name[0] = '0' + idx / 10;
        name[1] = '0' + (idx % 10);
        name[2] = '\0';
    }
    pts->miso = cirbuf_create(PTS_BUFSIZE);
    pts->mosi = cirbuf_create(PTS_BUFSIZE);
    pts->dev = device_fno_init(&mod_devpts, name, fno_search("/dev/pts"), FL_TTY, pts);
    pts->master = pty;
    pty->idx = idx;
    pty->fno = fno_create(&mod_devpty, "", &PTMX.PTMX_ROOT); 
    pty->fno->flags |= FL_TTY;
    pty->fno->priv = pty;
    pty->slave = pts;
    pty->task = NULL;
    pts->task = NULL;
    pty->sid = -1;
    pts->sid = -1;
    pty->creator_pid = this_task_getpid();
    return task_filedesc_add(pty->fno);
}

static void ptmx_create(void) 
{
    char name[5] = "ptmx";
    struct fnode *devfs = fno_search("/dev");
    if (!devfs)
        return;
    fno_mkdir(&mod_devpts, "pts", devfs);
    PTMX.dev = device_fno_init(&mod_ptmx, name, devfs, FL_TTY, &PTMX);
    PTMX.pty_idx = 0;
    memset(&PTMX.PTMX_ROOT, 0, sizeof(struct fnode));
}

int ptmx_init(void)
{
    ptmx_create();
    return 0;
}

int pty_open(const char *path, int flags)
{
    return -EINVAL;
}

int ptmx_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    struct fnode *master, *slave;
    if (f != PTMX.dev->fno)
        return -EINVAL;
    return pts_create();
}

static int pty_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct devpty *pty = NULL;
    struct devpts *pts = NULL;
    int ret = -EINVAL;
    if (len == 0)
        return 0;
    pty = (struct devpty *)FNO_MOD_PRIV(fno, &mod_devpty);
    if (pty)
        pts = pty->slave;
    if (pts) {
        mutex_lock(pts->dev->mutex);
        if (pts->master != pty)
            ret = -EPIPE;
        if (cirbuf_bytesinuse(pts->miso) > 0) {
            ret = cirbuf_readbytes(pts->miso, buf, len);
            if ((ret > 0) && pts->task)
                task_resume(pts->task);
        } else {
            pty->task = this_task();
            task_suspend();
            ret = SYS_CALL_AGAIN;
        }
        mutex_unlock(pts->dev->mutex);
    }
    return ret;
}

static int pty_write(struct fnode *fno, const void *buf, unsigned int len)
{
    struct devpty *pty = NULL;
    struct devpts *pts = NULL;
    int ret = -EINVAL;
    pty = (struct devpty *)FNO_MOD_PRIV(fno, &mod_devpty);
    if (pty)
        pts = pty->slave;
    if (pts) {
        mutex_lock(pts->dev->mutex);
        if (pts->master != pty)
            ret = -EPIPE;
        if (cirbuf_bytesfree(pts->mosi) > 0) {
            ret = cirbuf_writebytes(pts->mosi, buf, len);
            if ((ret > 0) && pts->task)
                task_resume(pts->task);
        } else {
            pty->task = this_task();
            task_suspend();
            ret = SYS_CALL_AGAIN;
        }
        mutex_unlock(pts->dev->mutex);
    }
    return ret;
}

static int pty_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    struct devpty *pty = NULL;
    struct devpts *pts = NULL;
    int ret = 0;
    pty = (struct devpty *)FNO_MOD_PRIV(fno, &mod_devpty);

    if (!pty)
        return -EINVAL;
    if (pty)
        pts = pty->slave;

    if (!pts)
        return -EPIPE;

    mutex_lock(pts->dev->mutex);
    if (pts->master != pty)
        ret = -EPIPE;
    if ((events & POLLOUT) && (cirbuf_bytesfree(pts->mosi) > 0)) {
        *revents |= POLLOUT;
        ret = 1;
    }
    if ((events & POLLIN) && (cirbuf_bytesinuse(pts->miso) > 0)) {
        *revents |= POLLIN;
        ret = 1;
    }
    if (ret == 0) {
        pty->task = this_task();
    }
    mutex_unlock(pts->dev->mutex);
    return ret;
}

static void pty_tty_attach(struct fnode *fno, int pid)
{
    struct devpty *pty = NULL;
    struct devpts *pts = NULL;
    int ret = 0;
    pty = (struct devpty *)FNO_MOD_PRIV(fno, &mod_devpty);
    if (pty->sid != pid) {
        pty->sid = pid;
    }
}

static int pty_close(struct fnode *fno)
{
    struct devpty *pty = NULL;
    struct devpts *pts = NULL;
    int ret = 0;
    pty = (struct devpty *)FNO_MOD_PRIV(fno, &mod_devpty);
    if (!pty)
        return -1;
    pts = pty->slave;
    if (!pts)
        return -1;

    if (this_task_getpid() == pty->creator_pid) {
        mutex_lock(pts->dev->mutex);
        fno_unlink(pts->dev->fno);
        pty->fno->priv = NULL;
        pts->dev->fno->priv = NULL;
        kfree(pty);
        pts->master = NULL;
        mutex_unlock(pts->dev->mutex);
        kfree(pts);
    }
    return 0;
}


int pts_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    struct fnode *master, *slave;
    if (f != PTMX.dev->fno)
        return -EINVAL;
    return pts_create();
}

static int pts_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct devpty *pty = NULL;
    struct devpts *pts = NULL;
    int ret = -EINVAL;
    if (len == 0)
        return 0;
    pts = (struct devpts *)FNO_MOD_PRIV(fno, &mod_devpts);
    if (!pts)
        goto out;
    mutex_lock(pts->dev->mutex);
    pty = pts->master;
    if (!pty) {
        ret = -EPIPE;
        goto out;
    }
    if (cirbuf_bytesinuse(pts->mosi) > 0) {
        ret = cirbuf_readbytes(pts->mosi, buf, len);
        if ((ret > 0) && pty->task)
            task_resume(pty->task);
    } else {
        pts->task = this_task();
        task_suspend();
        ret = SYS_CALL_AGAIN;
    }
out:
    mutex_unlock(pts->dev->mutex);
    return ret;
}

static int pts_write(struct fnode *fno, const void *buf, unsigned int len)
{
    struct devpty *pty = NULL;
    struct devpts *pts = NULL;
    int ret = -EINVAL;
    pts = (struct devpts *)FNO_MOD_PRIV(fno, &mod_devpts);
    if (!pts)
        goto out;
    mutex_lock(pts->dev->mutex);
    pty = pts->master;
    if (!pty) {
        ret = -EPIPE;
        goto out;
    }
    if (cirbuf_bytesfree(pts->miso) > 0) {
        ret = cirbuf_writebytes(pts->miso, buf, len);
        if ((ret > 0) && pty->task)
            task_resume(pty->task);
    } else {
        pts->task = this_task();
        task_suspend();
        ret = SYS_CALL_AGAIN;
    }
out:
    mutex_unlock(pts->dev->mutex);
    return ret;
}

static int pts_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    struct devpty *pty = NULL;
    struct devpts *pts = NULL;
    int ret = -EINVAL;
    pts = (struct devpts *)FNO_MOD_PRIV(fno, &mod_devpts);
    if (!pts)
        goto out;
    mutex_lock(pts->dev->mutex);
    pty = pts->master;
    if (!pty) {
        ret = -EPIPE;
        goto out;
    }

    ret = 0;

    if ((events & POLLOUT) && (cirbuf_bytesfree(pts->miso) > 0)) {
        *revents |= POLLOUT;
        ret = 1;
    }
    if ((events & POLLIN) && (cirbuf_bytesinuse(pts->mosi) > 0)) {
        *revents |= POLLIN;
        ret = 1;
    }
    if (ret == 0) {
        pts->task = this_task();
    }
out:
    mutex_unlock(pts->dev->mutex);
    return ret;
}

static void pts_tty_attach(struct fnode *fno, int pid)
{
    struct devpts *pts = NULL;
    int ret = 0;
    pts = (struct devpts *)FNO_MOD_PRIV(fno, &mod_devpts);
    if (pts->sid != pid) {
        pts->sid = pid;
    }
}

