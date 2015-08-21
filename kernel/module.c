#include "frosted.h"

struct address_family {
    struct module *mod;
    uint16_t family; 
    struct address_family *next;
};

struct module *MODS = NULL;
struct address_family *AF = NULL;

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

static struct module *af_to_module(uint16_t family)
{
    struct address_family *af = AF;
    while (af) {
        if (af->family == family)
            return af->mod;
        af = af->next;
    }
    return NULL;
}

int register_addr_family(struct module *m, uint16_t family)
{
   struct address_family *af;
   if (af_to_module(family))
       return -1; /* Another module already claimed this AF */
   af = kalloc(sizeof(struct address_family));
   if (!af)
       return -1;
   af->family = family;
   af->mod = m;
   af->next = AF;
   AF = af;
   return 0;
} 

int sys_read_hdlr(int fd, void *buf, int len)
{
    struct fnode *fno = task_filedesc_get(fd);
    if (fno) {
        return fno->owner->ops.read(fd, buf, len);
    }
    return -1;
}

int sys_write_hdlr(int fd, void *buf, int len)
{
    struct fnode *fno = task_filedesc_get(fd);
    if (fno && fno->owner && fno->owner->ops.write) {
        return fno->owner->ops.write(fd, buf, len);
    }
    return -1;
}

int sys_socket_hdlr(int family, int type, int proto)
{
    struct module *m = af_to_module(family);
    if(!m || !(m->ops.socket))
        return -1;
    return m->ops.socket(family, type, proto);
}
