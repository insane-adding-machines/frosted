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
 *      Authors:
 *
 */

#include "frosted.h"
#include "string.h"

static struct fnode *devnull;
static struct fnode *devzero;


static int devnull_read(struct fnode *fno, void *buf, unsigned int len)
{
    if (fno == devnull)
        return -EPERM;
    if (len <= 0)
        return len;
    memset(buf, 0, sizeof(len));
    return (int)len;
}


static int devnull_write(struct fnode *fno, const void *buf, unsigned int len)
{
    if (fno == devzero)
        return -EPERM;
    if (len <= 0)
        return len;
    return len;
}

static int devnull_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    return 1;
}

static int devnull_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    return task_filedesc_add(f);

}



static struct module mod_devnull = {
};


void devnull_init(struct fnode *dev)
{
    strcpy(mod_devnull.name,"devnull");
    mod_devnull.family = FAMILY_FILE;
    mod_devnull.ops.open = devnull_open;
    mod_devnull.ops.read = devnull_read;
    mod_devnull.ops.poll = devnull_poll;
    mod_devnull.ops.write = devnull_write;

    devnull = fno_create_wronly(&mod_devnull, "null", dev);
    devnull->flags |= FL_TTY;
    devzero = fno_create_rdonly(&mod_devnull, "zero", dev);
    devzero->flags |= FL_TTY;
    register_module(&mod_devnull);
}
