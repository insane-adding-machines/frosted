#ifndef _FROSTED_POLL_H
#define _FROSTED_POLL_H

#include <stdint.h>
#include <sys/types.h>

/* poll */
#define POLLIN		0x0001
#define POLLPRI		0x0002
#define POLLOUT		0x0004
#define POLLERR		0x0008
#define POLLHUP		0x0010
#define POLLNVAL	0x0020

struct pollfd {
    int fd;
    uint16_t events;
    uint16_t revents;
};

/* for select */
typedef unsigned int nfds_t;

#endif /* _FROSTED_POLL_H */

