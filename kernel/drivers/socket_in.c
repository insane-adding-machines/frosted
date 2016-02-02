#include "frosted.h"
#include "socket_in.h"
#include <string.h>
#include <sys/types.h>

static struct module mod_socket_in;

struct frosted_inet_socket {
    struct fnode *node;
    struct pico_socket *sock;
    uint16_t pid;
    int fd;
    uint16_t events;
    uint16_t revents;
    int bytes;
};

static int sock_check_fd(int fd, struct fnode **fno)
{
    *fno = task_filedesc_get(fd);
    
    if (!fno)
        return -1;

    if (fd < 0)
        return -1;
    if ((*fno)->owner != &mod_socket_in)
        return -1;

    return 0;
}

static int sock_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    *revents = events;
    return 1;
}

static struct frosted_inet_socket *fd_inet(int fd)
{
    struct fnode *fno;
    struct frosted_inet_socket *s;
    if (!fno)
        return NULL;

    if (sock_check_fd(fd, &fno) != 0)
        return NULL;

    s = (struct frosted_inet_socket *)fno->priv;
    return s;
}

static struct frosted_inet_socket *pico_inet(struct pico_socket *s)
{
    return (struct frosted_inet_socket *)(s->priv);
}

static void pico_socket_event(uint16_t ev, struct pico_socket *sock)
{
    struct frosted_inet_socket *s;
    s = pico_inet(sock);
    if (!s) {
        return;
    }
    s->revents |= ev;
    if ((s->revents & s->events) != 0)
        task_resume(s->pid);
}

static int sock_close(struct fnode *fno)
{
    struct frosted_inet_socket *s;
    if (!fno)
        return -1;
    s = (struct frosted_inet_socket *)fno->priv;
    if (!s)
        return -1;
    pico_socket_close(s->sock);
    kprintf("## Closed INET socket!\n");
    kfree((struct fnode *)s->node);
    kfree(s);
    return 0;
}

static struct frosted_inet_socket *inet_socket_new(void)
{
    struct frosted_inet_socket *s;
    s = kcalloc(sizeof(struct frosted_inet_socket), 1);
    if (!s)
        return NULL;
    s->node = kcalloc(sizeof(struct fnode), 1);
    if (!s->node) {
        kfree(s);
        return NULL;
    }
    return s;
}

int sock_socket(int domain, int type, int protocol)
{
    int fd = -1;
    struct frosted_inet_socket *s;

    s = inet_socket_new();
    if (!s)
        return -ENOMEM;

    s->sock = pico_socket_open(domain, type, pico_socket_event);
    if (!s->sock) {
        kfree((struct fnode *)s->node);
        kfree(s);
        return 0 - pico_err;
    }
    s->node->owner = &mod_socket_in;
    s->node->priv = s;
    s->sock->priv = s;
    kprintf("## Open INET socket!\n");
    s->fd = task_filedesc_add(s->node);
    return fd;
}

int sock_recvfrom(int fd, void *buf, unsigned int len, int flags, struct sockaddr *addr, unsigned int *addrlen)
{
    struct frosted_inet_socket *s;
    int ret;
    uint16_t port;
    struct pico_ip4 paddr;

    s = fd_inet(fd);
    if (!s)
        return -EINVAL;
    while (s->bytes < len) {
        if ((addr) && ((*addrlen) > 0))
            ret = pico_socket_recvfrom(s->sock, buf + s->bytes, len - s->bytes, &paddr, &port);
        else
            ret = pico_socket_recvfrom(s->sock, buf + s->bytes, len - s->bytes, NULL, NULL);

        if (ret < 0)
            return 0 - pico_err;
        if (ret == 0) {
            s->events = PICO_SOCK_EV_RD;
            task_suspend();
            return SYS_CALL_AGAIN;
        }
        s->bytes += ret;
        if ((s->sock->proto->proto_number) == PICO_PROTO_UDP && (s->bytes > 0))
            break;
    }
    if (addr) {
        ((struct sockaddr_in *)addr)->sin_family = AF_INET;
        ((struct sockaddr_in *)addr)->sin_port = port;
        ((struct sockaddr_in *)addr)->sin_addr.s_addr = paddr.addr;
    }
    s->bytes = 0;
    s->events  &= (~PICO_SOCK_EV_RD);
    s->revents &= (~PICO_SOCK_EV_RD);
    return ret;
}

int sock_sendto(int fd, const void *buf, unsigned int len, int flags, struct sockaddr *addr, unsigned int addrlen)
{
    struct frosted_inet_socket *s;
    uint16_t port;
    struct pico_ip4 paddr;
    int ret;
    s = fd_inet(fd);
    if (!s)
        return -EINVAL;

    while (len > s->bytes) {
        if ((addr) && (addrlen >0))
        {
            paddr.addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
            port = ((struct sockaddr_in *)addr)->sin_port;
            ret = pico_socket_sendto(s->sock, buf + s->bytes, len - s->bytes, &paddr, port); 
        } else {
            ret = pico_socket_sendto(s->sock, buf + s->bytes, len - s->bytes, NULL, 0); 
        }
        if (ret == 0) {
            s->events = PICO_SOCK_EV_WR;
            task_suspend();
            return SYS_CALL_AGAIN;
        }

        if (ret < 0)
            return (0 - pico_err);

        s->bytes += ret;
        if ((s->sock->proto->proto_number) == PICO_PROTO_UDP && (s->bytes > 0))
            break;
    }
    s->bytes = 0;
    s->events  &= (~PICO_SOCK_EV_WR);
    s->revents &= (~PICO_SOCK_EV_WR);
    return ret;
}

int sock_bind(int fd, struct sockaddr *addr, unsigned int addrlen)
{
    struct frosted_inet_socket *s;
    union pico_address paddr;
    uint16_t port;
    int ret;
    s = fd_inet(fd);
    if (!s)
        return -EINVAL;
    paddr.ip4.addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
    port = ((struct sockaddr_in *)addr)->sin_port;
    ret = pico_socket_bind(s->sock, &paddr, &port);
    if (ret > 0) {
        ((struct sockaddr_in *)addr)->sin_port = port;
        ret = 0;
    }
    return 0 - pico_err;
}

int sock_accept(int fd, struct sockaddr *addr, unsigned int *addrlen)
{
    struct frosted_inet_socket *l, *s;
    struct pico_socket *cli;
    union pico_address paddr; 
    uint16_t port;

    l = fd_inet(fd);
    if (!l)
        return -EINVAL;
    l->events = PICO_SOCK_EV_CONN;

    cli = pico_socket_accept(l->sock, &paddr, &port);
    if ((cli == NULL) && (pico_err != PICO_ERR_EAGAIN))
        return 0 - pico_err;

    if (cli) {
        s = inet_socket_new();
        if (!s) {
            pico_socket_close(cli);
            return -ENOMEM;
        }
        s->sock = cli;
        s->node->owner = &mod_socket_in;
        s->node->priv = s;
        s->sock->priv = s;
        s->fd = task_filedesc_add(s->node);
        return s->fd;
    } else {
        task_suspend();
        return SYS_CALL_AGAIN;
    }
}

int sock_connect(int fd, struct sockaddr *addr, unsigned int addrlen)
{
    struct frosted_inet_socket *s;
    int ret;
    union pico_address paddr;
    uint16_t port;
    s = fd_inet(fd);
    if (!s)
        return -EINVAL;
    s->events = PICO_SOCK_EV_CONN;
    if ((s->revents & PICO_SOCK_EV_CONN) == 0) {
        paddr.ip4.addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
        port = ((struct sockaddr_in *)addr)->sin_port;
        ret = pico_socket_connect(s->sock, &paddr, port);
        task_suspend();
        return SYS_CALL_AGAIN;
    }
    s->events  &= (~PICO_SOCK_EV_CONN);
    s->revents &= (~PICO_SOCK_EV_CONN);
    return ret;
}

int sock_listen(int fd, int backlog)
{
    struct frosted_inet_socket *s;
    s = fd_inet(fd);
    if (!s)
        return -EINVAL;
    return pico_socket_listen(s->sock, backlog);
}

int sock_shutdown(int fd, uint16_t how)
{
    struct frosted_inet_socket *s;
    s = fd_inet(fd);
    if (!s)
        return -EINVAL;
    return pico_socket_shutdown(s->sock, how);
}

void socket_in_init(void)
{
    mod_socket_in.family = FAMILY_INET;
    strcpy(mod_socket_in.name,"picotcp");
    mod_socket_in.ops.poll = sock_poll;
    mod_socket_in.ops.close = sock_close;

    mod_socket_in.ops.socket     = sock_socket;
    mod_socket_in.ops.connect    = sock_connect;
    mod_socket_in.ops.accept     = sock_accept;
    mod_socket_in.ops.bind       = sock_bind;
    mod_socket_in.ops.listen     = sock_listen;
    mod_socket_in.ops.recvfrom   = sock_recvfrom;
    mod_socket_in.ops.sendto     = sock_sendto;
    mod_socket_in.ops.shutdown   = sock_shutdown;

    register_module(&mod_socket_in);
    register_addr_family(&mod_socket_in, FAMILY_INET);
}



