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

int sys_poll_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    struct pollfd *pfd = (struct pollfd *)arg1;
    int i, n = (int)arg2;
    uint32_t timeout = jiffies + arg3;
    int ret = 0;
    struct fnode *f;

    /* TODO: Set process wakeup timer */

    while (jiffies < timeout) {
        for (i = 0; i < n; i++) {
            f = task_filedesc_get(pfd[i].fd);
            if (!f || !f->owner || !f->owner->ops.poll) {
                return -EOPNOTSUPP;
            }
            ret += f->owner->ops.poll(f, pfd[i].events, &pfd[i].revents);
        }
        if (ret > 0)
            return ret;
        task_suspend();
        return SYS_CALL_AGAIN;
    }
    return 0;
}
