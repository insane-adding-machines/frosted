#include "frosted.h"

struct module *MODS = NULL;


int register_module(struct module *m)
{
    m->next = MODS;
    MODS = m;
    return 0;
}

int unregister_module(struct module *m)
{
    struct module *cur = MODS;
    while (cur) {
        /*XXX*/
        cur = cur->next;
    }
}

int sys_read_hdlr(int fd, void *buf, int len)
{
    struct fnode *fno = fno_get(fd);
    if (fno) {
        return fno->owner->ops.read(fd, buf, len);
    }
    return -1;
}

int sys_write_hdlr(int fd, void *buf, int len)
{
    struct fnode *fno = fno_get(fd);
    if (fno) {
        return fno->owner->ops.write(fd, buf, len);
    }
    return -1;
}
