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
#include "unicore-mx/cm3/systick.h"

#define MAX_TASKLETS 64

struct tasklet {
    void (*exe)(void *);
    void *arg;
};

static struct tasklet tasklet_array[MAX_TASKLETS];
uint32_t n_tasklets = 0;
uint32_t max_tasklets = 0;


void tasklet_add(void (*exe)(void*), void *arg)
{
    int i;
    irq_off();
    for (i = 0; i < MAX_TASKLETS; i++) {
        if (!tasklet_array[i].exe) {
            tasklet_array[i].exe = exe;
            tasklet_array[i].arg = arg;
            n_tasklets++;
            if (n_tasklets > max_tasklets)
                max_tasklets = n_tasklets;
            irq_on();
            return;
        }
    }
    while(1) { /* Too many tasklets. */; }

}

void check_tasklets(void)
{
    int i;
    if (n_tasklets == 0)
        return;
    irq_off();
    for (i = 0; i < MAX_TASKLETS; i++) {
        if (tasklet_array[i].exe) {
            tasklet_array[i].exe(tasklet_array[i].arg);
            tasklet_array[i].exe = NULL;
            tasklet_array[i].arg = NULL;
            n_tasklets--;
        }
    }
    irq_on();
}
