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
#include "drivers/socket_in.h"

#ifndef CONFIG_DNS_CLIENT
int sys_getaddrinfo_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    return -ENOSYS;
}

int sys_freeaddrinfo_hdlr(uint32_t arg1)
{
    return -ENOSYS;
}
#else

#define DNSQUERY_IDLE           0
#define DNSQUERY_OK             1
#define DNSQUERY_IN_PROGRESS    2
#define DNSQUERY_FAIL           0xFF

struct dnsquery_cookie
{
    char            *node;
    struct addrinfo *res;
    struct task     *task;
    uint8_t         state;
};

struct waiting_task {
    struct task *task;
    struct waiting_task *next;
};




static struct dnsquery_cookie ck4 = { NULL, 0, DNSQUERY_IDLE };
static struct dnsquery_cookie ck6 = { NULL, 0, DNSQUERY_IDLE };
static struct waiting_task *waiting_list = NULL;


static int dns_is_busy(void)
{
    if (ck4.state == DNSQUERY_IN_PROGRESS)
        return 1;
#ifdef CONFIG_IPV6
    if (ck6.state == DNSQUERY_IN_PROGRESS)
        return 1;
#endif
    return 0;
}

static int add_to_waiting_list(void)
{
    struct waiting_task *wt = kalloc(sizeof(struct waiting_task));
    if (!wt)
        return -1;
    wt->next = waiting_list;
    wt->task = this_task();
    return 0;
}

#ifdef CONFIG_IPV6
static void dns_ip6_cb(char *ip, void *arg)
{
    if (ip) {
        new = f_calloc(MEM_USER, 1, sizeof(struct addrinfo));
        if (!new) {
            ck6.state= DNSQUERY_FAIL;
            task_resume(ck6.task);
            return;
        }
        new->ai_family = AF_INET6;
        new->ai_addr = f_calloc(MEM_USER, 1, sizeof(struct sockaddr_in6));
        if (!new->ai_addr) {
            f_free(new);
            ck6.state = DNSQUERY_FAIL;
            task_resume(ck6.task);
            return;
        }
        new->ai_addrlen = sizeof(struct sockaddr_in6);
        pico_string_to_ipv6(ip, (((struct sockaddr_in6*)(new->ai_addr))->sin6_addr.s6_addr)); 
        ((struct sockaddr_in6*)(new->ai_addr))->sin6_family = AF_INET6;
        new->ai_next = ck6.res;
        ck6.res = new;
        ck6.state = DNSQUERY_OK;
    } else {
        /* No ip given, but still callback was called: timeout! */
        ck6.state = DNSQUERY_FAIL;
    }
    task_resume(ck6.task);
}
#endif

static void dns_ip4_cb(char *ip, void *arg)
{
    struct addrinfo *new;
    if (ip) {
        new = f_calloc(MEM_USER, 1, sizeof(struct addrinfo));
        if (!new) {
            ck4.state = DNSQUERY_FAIL;
            task_resume(ck4.task);
            return;
        }
        new->ai_family = AF_INET;
        new->ai_addr = f_calloc(MEM_USER, 1, sizeof(struct sockaddr_in));
        if (!new->ai_addr) {
            f_free(new);
            ck4.state = DNSQUERY_FAIL;
            task_resume(ck4.task);
            return;
        }
        new->ai_addrlen = sizeof(struct sockaddr_in);
        pico_string_to_ipv4(ip, &(((struct sockaddr_in*)new->ai_addr)->sin_addr.s_addr));    
        ((struct sockaddr_in*)(new->ai_addr))->sin_family = AF_INET;
        new->ai_next = ck4.res;
        ck4.res = new;
        ck4.state = DNSQUERY_OK;
    } else {
        /* No ip given, but still callback was called: timeout! */
        ck4.state = DNSQUERY_FAIL;
    }
    task_resume(ck4.task);
}

static void dns_idle(void)
{
    struct waiting_task *wt;
    ck6.state = DNSQUERY_IDLE;
    ck4.state = DNSQUERY_IDLE;

    while(waiting_list) {
        wt = waiting_list;
        task_resume(wt->task);
        waiting_list = wt->next;
        kfree(wt);
    }
}

static int pico_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
    struct sockaddr_in sa4;
    struct sockaddr_in6 sa6;
    (void)service;

    /* If an IP address was passed in the string, resolve immediately */
#ifdef CONFIG_IPV6
    if (pico_string_to_ipv6(node, sa6.sin6_addr.s6_addr) == 0) {
        struct addrinfo *ai = f_calloc(MEM_USER, 1, sizeof(struct addrinfo));
        if (!ai)
            return -ENOMEM;
        ai->ai_family = AF_INET6;
        ai->ai_addr = f_calloc(MEM_USER, 1, sizeof(struct sockaddr_in6));
        ai->ai_addrlen = sizeof(struct sockaddr_in6);
        ai->ai_next = NULL;
        memcpy(ai->ai_addr, &sa6, sizeof(struct sockaddr_in6));
        return 0;
    }
#endif

    if (pico_string_to_ipv4(node, &sa4.sin_addr.s_addr) == 0) {
        struct addrinfo *ai = f_calloc(MEM_USER, 1, sizeof(struct addrinfo));
        if (!ai)
            return -ENOMEM;
        ai->ai_family = AF_INET;
        ai->ai_addr = f_calloc(MEM_USER, 1, sizeof(struct sockaddr_in));
        ai->ai_addrlen = sizeof(struct sockaddr_in);
        ai->ai_next = NULL;
        memcpy(ai->ai_addr, &sa4, sizeof(struct sockaddr_in));
        return 0;
    }

    /* If an operation is in progress, block until it's over */ 
    if (dns_is_busy()) {
        if (add_to_waiting_list() < 0)
            return -ENOMEM;
        task_suspend();
        return SYS_CALL_AGAIN;
    }

    /* Check if it's the resumed call, and the address has been resolved. */
#ifdef CONFIG_IPV6 
    if ((ck6.state == DNSQUERY_OK) && (ck6.node == node)) {
        *res = ck6.res;
        dns_idle();
        return 0;
    }
    
    if ((ck6.state == DNSQUERY_FAIL) && (ck4.node == node)) {
        dns_idle();
        return 0 - pico_err;
    }
#endif

    if ((ck4.state == DNSQUERY_OK) && (ck4.node == node)) {
        *res = ck4.res;
        dns_idle();
        return 0;
    }

    if ((ck4.state == DNSQUERY_FAIL) && (ck4.node == node)) {
        dns_idle();
        return 0 - pico_err;
    }

#ifdef CONFIG_IPV6
    if (!hints || (hints->ai_family == AF_INET6)) {
        ck6.state = DNSQUERY_IN_PROGRESS; 
        ck6.node = node;
        ck6.res = NULL;
        ck6.task = this_task();
        if (pico_dns_client_getaddr6(node, dns_ip6_cb, NULL) < 0) {
            dns_idle();
            return 0 - pico_err;
        }
    }
#endif

    if (!hints || (hints->ai_family == AF_INET)) {
        ck4.state = DNSQUERY_IN_PROGRESS; 
        ck4.node = node;
        ck4.res = NULL;
        ck4.task = this_task();
        if (pico_dns_client_getaddr(node, dns_ip4_cb, NULL) < 0) {
            dns_idle();
            return 0 - pico_err;
        }
    }
    task_suspend();
    return SYS_CALL_AGAIN;
}

static int pico_freeaddrinfo(struct addrinfo *res)
{
    struct addrinfo *cur = res;
    struct addrinfo *nxt;
    while(cur) {
        if (cur->ai_addr)
            f_free(cur->ai_addr);
        nxt = cur->ai_next;
        f_free(cur);
        cur = nxt;
    }
    return 0;
}


int sys_getaddrinfo_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    if (task_ptr_valid(arg1) || task_ptr_valid(arg2) || task_ptr_valid(arg3) || task_ptr_valid(arg4))
        return -EACCES;
    return pico_getaddrinfo((const char *)arg1, (const char *)arg2, (struct addrinfo *)arg3, (struct addrinfo **)arg4);
}

int sys_freeaddrinfo_hdlr(uint32_t arg1)
{
    if (task_ptr_valid(arg1))
        return -EACCES;
    return pico_freeaddrinfo((struct addrinfo *)arg1);
}

#endif
