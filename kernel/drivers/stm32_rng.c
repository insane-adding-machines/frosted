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
 *      Authors: brabo
 *
 */

#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "stm32_rng.h"

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/rng.h>
#define CLOCK_ENABLE(C)                 rcc_periph_clock_enable(C);

struct dev_rng {
	struct device * dev;
	uint32_t base;
};

#define MAX_RNGS 1

static struct dev_rng DEV_RNG[MAX_RNGS];

static int devrng_read(struct fnode *fno, void *buf, unsigned int len);

static struct module mod_devrng = {
	.family = FAMILY_FILE,
	.name = "random",
	.ops.open = device_open,
	.ops.read = devrng_read,
};

static int devrng_read(struct fnode *fno, void *buf, unsigned int len)
{
	struct dev_rng *rng;

	if (len == 0)
		return len;

	rng = (struct dev_rng *)FNO_MOD_PRIV(fno, &mod_devrng);

	if (!rng)
		return -1;

	mutex_lock(rng->dev->mutex);

	/* need to sort this out */
	uint32_t error_bits = 0;
	error_bits = RNG_SR_SEIS | RNG_SR_CEIS;

	if (((RNG_SR & error_bits) != 0) ||
		    ((RNG_SR & RNG_SR_DRDY) != 1)) {
        	rng->dev->pid = scheduler_get_cur_pid();
        	task_suspend();
        	mutex_unlock(rng->dev->mutex);
        	return SYS_CALL_AGAIN;
	}

	rng_get_random((uint32_t *) buf);

	mutex_unlock(rng->dev->mutex);
	return 4;
}

static void rng_fno_init(struct fnode *dev, uint32_t n, const struct rng_addr * addr)
{
	struct dev_rng *r = &DEV_RNG[n];
	r->dev = device_fno_init(&mod_devrng, mod_devrng.name, dev, FL_RDONLY, r);
	r->base = addr->base;
}

void rng_init(struct fnode * dev,  const struct rng_addr rng_addrs[], int num_rngs)
{
	int i;
	for (i = 0; i < num_rngs; i++)
	{
		if (rng_addrs[i].base == 0)
			continue;

		rng_fno_init(dev, i, &rng_addrs[i]);
		CLOCK_ENABLE(rng_addrs[i].rcc);

		rng_enable();
	}
	register_module(&mod_devrng);
}

