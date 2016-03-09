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
#include <string.h>
#include <sys/termios.h>

static struct module *get_term_mod(int td)
{
    struct fnode *f = task_filedesc_get(td);
    if (f)
        return f->owner;
    return NULL;
}

int sys_tcgetattr_hdlr(int arg1, int arg2)
{
    struct termios *t = (struct termios *)arg2;
    struct module *m;
    m = get_term_mod(arg1);
    if (m && m->ops.tcgetattr)
        return m->ops.tcgetattr(arg1, t);
    else
        return -EOPNOTSUPP;
}

int sys_tcsetattr_hdlr(int arg1, int arg2, int arg3)
{
    const struct termios *t = (const struct termios *)arg3;
    struct module *m;
    m = get_term_mod(arg1);
    if (m && m->ops.tcsetattr)
        return m->ops.tcsetattr(arg1, arg2, t);
    else
        return -EOPNOTSUPP;
}


int sys_tcsendbreak_hdlr(int arg1, int arg2)
{
    /* TODO: send SIGINT to self. */
    return -EOPNOTSUPP;
}
