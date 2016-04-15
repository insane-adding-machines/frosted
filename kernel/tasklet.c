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

struct tasklet {
    void (*exe)(void *);
    void *arg;
    struct tasklet *next;
};

struct tasklet *tasklet_list_head = NULL;
struct tasklet *tasklet_list_tail = NULL;


void tasklet_add(void (*exe)(void*), void *arg)
{
    struct tasklet *t, *x;
    irq_off();
    t = kalloc(sizeof(struct tasklet));
    x = tasklet_list_tail;
    if  (!t)
        return;
    t->exe = exe;
    t->arg = arg;
    t->next = NULL;
    if (!x) {
        tasklet_list_head = t;
        tasklet_list_tail = t;
    } else {
        x->next = t;
        tasklet_list_tail = t;
    }
    systick_counter_enable();
    irq_on();
}

void check_tasklets(void)
{
    struct tasklet *t, *n;
    irq_off();
    t = tasklet_list_head;
    tasklet_list_head = NULL;
    tasklet_list_tail = NULL;
    irq_on();
    while(t) {
        n = t->next;
        if (t->exe) {
            t->exe(t->arg);
        }
        //memset(t, 0x0a, sizeof(struct tasklet)); /* For testing... */
        kfree(t);
        t = n;
    }
}
