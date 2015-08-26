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
                return -1;
            }
            ret += f->owner->ops.poll(pfd[i].fd, pfd[i].events, &pfd[i].revents);
        }
        if (ret > 0)
            return ret;
        task_suspend();
        return SYS_CALL_AGAIN;
    }
    return 0;
}
