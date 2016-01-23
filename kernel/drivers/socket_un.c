#include "frosted.h"
#include <string.h>

static struct module mod_socket_un;
struct fnode FNO_SOCKUN_STUB = {
    .owner = &mod_socket_un
};

static int sock_check_fd(int fd, struct fnode **fno)
{
    *fno = task_filedesc_get(fd);
    
    if (!fno)
        return -1;

    if (fd < 0)
        return -1;
    if ((*fno)->owner != &mod_socket_un)
        return -1;

    return 0;
}

static int sock_poll(int fd, uint16_t events, uint16_t *revents)
{
    *revents = events;
    return 1;
}


static int sock_close(int fd)
{
    struct fnode *fno;
    if (sock_check_fd(fd, &fno))
        return -1;
    kprintf("## Closed UNIX socket!\n");
    /* TODO */
    return 0;
}

int sock_socket(int domain, int type, int protocol)
{
    int fd = -1;
    kprintf("## Opened UNIX socket!\n");
    fd = task_filedesc_add(&FNO_SOCKUN_STUB);
    return fd;
}
int sock_recvfrom(int fd, void *buf, unsigned int len, int flags, struct sockaddr *addr, unsigned int *addrlen)
{
    return -1;
}

int sock_sendto(int fd, const void *buf, unsigned int len, int flags, struct sockaddr *addr, unsigned int addrlen)
{
    return -1;
}
int sock_bind(int fd, struct sockaddr *addr, unsigned int addrlen)
{
    return -1;
}

int sock_accept(int fd, struct sockaddr *addr, unsigned int *addrlen)
{
    return -1;
}

int sock_connect(int fd, struct sockaddr *addr, unsigned int addrlen)
{
    return -1;
}

int sock_listen(int fd, int backlog)
{
    return -1;
}

int sock_shutdown(int fd, uint16_t how)
{
    return -1;
}


void socket_un_init(void)
{
    mod_socket_un.family = FAMILY_UNIX;
    strcpy(mod_socket_un.name,"un");
    mod_socket_un.ops.poll = sock_poll;
    mod_socket_un.ops.close = sock_close;

    mod_socket_un.ops.socket     = sock_socket;
    mod_socket_un.ops.connect    = sock_connect;
    mod_socket_un.ops.accept     = sock_accept;
    mod_socket_un.ops.bind       = sock_bind;
    mod_socket_un.ops.listen     = sock_listen;
    mod_socket_un.ops.recvfrom   = sock_recvfrom;
    mod_socket_un.ops.sendto     = sock_sendto;
    mod_socket_un.ops.shutdown   = sock_shutdown;

    register_module(&mod_socket_un);
    register_addr_family(&mod_socket_un, FAMILY_UNIX);
}



