/*********************************************************************
PicoTCP. Copyright (c) 2013 TASS Belgium NV. Some rights reserved.
See LICENSE and COPYING for usage.
Do not redistribute without a written permission by the Copyright
holders.

Author: Maxime Vincent, Daniele Lacamera
*********************************************************************/
#ifndef FROSTED_IN_SOCKETS_H_
#define FROSTED_IN_SOCKETS_H_

#include <stdint.h>
#include <signal.h>

#ifdef CONFIG_PICOTCP
#include "pico_defines.h"
#include "pico_constants.h"
#include "pico_config.h"
#include "pico_stack.h"
#include "pico_icmp4.h"
#include "pico_stack.h"
#include "pico_ipv4.h"
#include "pico_ipv6.h"
#include "pico_dns_client.h"
#include "pico_socket.h"
#endif

#define SOCKSIZE  16
#define SOCKSIZE6 28

struct pico_bsd_endpoint;
extern void   *picoLock;
extern void   *pico_signal_tick;


typedef int socklen_t;
#define AF_INET     (2)
#define AF_INET6    (10)
#define SOL_SOCKET (0x80)

#define IP_MULTICAST_LOOP   (PICO_IP_MULTICAST_LOOP)
#define IP_MULTICAST_TTL    (PICO_IP_MULTICAST_TTL)
#define IP_MULTICAST_IF     (PICO_IP_MULTICAST_IF)
#define IP_ADD_MEMBERSHIP   (PICO_IP_ADD_MEMBERSHIP)
#define IP_DROP_MEMBERSHIP  (PICO_IP_DROP_MEMBERSHIP)
#define SO_RCVBUF           (PICO_SOCKET_OPT_RCVBUF)
#define SO_SNDBUF           (PICO_SOCKET_OPT_SNDBUF)
#define TCP_NODELAY         (PICO_TCP_NODELAY)
#define TCP_KEEPCNT         (PICO_SOCKET_OPT_KEEPCNT)
#define TCP_KEEPIDLE        (PICO_SOCKET_OPT_KEEPIDLE)
#define TCP_KEEPINTVL       (PICO_SOCKET_OPT_KEEPINTVL)
#define TCP_LINGER          (PICO_SOCKET_OPT_LINGER)
#define SO_ERROR            (4103)
#define SO_REUSEADDR        (2)
#define IPPROTO_ICMP        (1)
#define sockopt_get_name(x) ((x))

#define INET_ADDRSTRLEN    (16)
#define INET6_ADDRSTRLEN   (46)

struct in_addr {
    uint32_t s_addr;
};

#define INADDR_ANY ((uint32_t)0U)

struct in6_addr {
    uint8_t s6_addr[16];
};

struct __attribute__((packed)) sockaddr_in {
    uint16_t sin_family;
    uint16_t sin_port;
    struct in_addr sin_addr;
    uint8_t _pad[SOCKSIZE - 8];         
};


struct __attribute__((packed)) sockaddr_in6 {
    uint16_t sin6_family;
    uint16_t sin6_port;
    uint32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t sin6_scope_id;
};

struct __attribute__((packed)) sockaddr_storage {
    uint16_t ss_family;
    uint8_t  _pad[(SOCKSIZE6 - sizeof(uint16_t))];
};

/* getaddrinfo */
struct addrinfo {
    int              ai_flags;
    int              ai_family;
    int              ai_socktype;
    int              ai_protocol;
    socklen_t        ai_addrlen;
    char            *ai_canonname;
    struct sockaddr *ai_addr;
    struct addrinfo *ai_next;
};


/* hostent */
struct hostent {
    char  *h_name;            /* official name of host */
    char **h_aliases;         /* alias list */
    int    h_addrtype;        /* host address type */
    int    h_length;          /* length of address */
    char **h_addr_list;       /* list of addresses */
};
#define h_addr h_addr_list[0] /* for backward compatibility */

/* fd_set */
#ifndef FD_SETSIZE
#define FD_SETSIZE  64 /* 64 files max, 1 bit per file -> 64bits = 8 bytes */
#endif

struct pico_fd_set_s {
    uint8_t fds_bits[FD_SETSIZE/8];
};

typedef struct pico_fd_set_s pico_fd_set;
#ifndef fd_set
#define fd_set pico_fd_set
#endif

//typedef void sigset_t;

#define   PICO_FD_SET(n, p)   ((p)->fds_bits[(n)/8] |=  (1u << ((n) % 8)))
#define   PICO_FD_CLR(n, p)   ((p)->fds_bits[(n)/8] &= ~(1u << ((n) % 8)))
#define   PICO_FD_ISSET(n, p) ((p)->fds_bits[(n)/8] &   (1u << ((n) % 8)))
#define   PICO_FD_ZERO(p)     do{memset((p)->fds_bits, 0, sizeof(struct pico_fd_set_s));}while(0)

/* Not socket related */
#ifndef __time_t_defined
typedef pico_time time_t;
#define __time_t_defined
#endif

#if !defined _TIME_H && !defined _TIMEVAL_DEFINED && !defined _STRUCT_TIMEVAL
struct timeval {
    time_t tv_sec;
    time_t tv_usec;
};

#if !defined __timespec_defined && !defined _SYS__TIMESPEC_H_
struct timespec {
    long tv_sec;
    long tv_nsec;
};
#endif

struct timezone {
    int tz_minuteswest;     /* minutes west of Greenwich */
    int tz_dsttime;         /* type of DST correction */
};
#define _TIMEVAL_DEFINED
#endif

#ifndef SO_REUSEPORT
    #define SO_REUSEPORT    (15)
#endif


#ifndef O_NONBLOCK
    #define O_NONBLOCK  0x4000
#endif

#ifndef _SYS_POLL_H
    #define POLLIN      0x001       /* There is data to read.  */
    #define POLLPRI     0x002       /* There is urgent data to read.  */
    #define POLLOUT     0x004       /* Writing now will not block.  */
    #define POLLRDNORM 0x040       /* Normal data may be read.  */
    #define POLLRDBAND 0x080       /* Priority data may be read.  */
    #define POLLWRNORM 0x100       /* Writing now will not block.  */
    #define POLLWRBAND 0x200       /* Priority data may be written.  */
    
    #define POLLMSG    0x400
    #define POLLREMOVE 0x1000
    #define POLLRDHUP  0x2000
    
    #define POLLERR     0x008       /* Error condition.  */
    #define POLLHUP     0x010       /* Hung up.  */
    #define POLLNVAL    0x020       /* Invalid polling request.  */
    
    typedef unsigned long int nfds_t;
    
    struct pollfd {
        int fd;
        uint16_t events;
        uint16_t revents;
    };
#endif

void pico_lock(void);
void pico_unlock(void);
void socket_in_init(void);

#endif /* PICO_BSD_SOCKETS_H_ */
