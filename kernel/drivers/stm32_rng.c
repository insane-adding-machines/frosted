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
#include "frand.h"
#include "frand_fortuna.h"
#include "frand_sha256.h"
#include <stdint.h>
#include "stm32_rng.h"

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/rng.h>
#define CLOCK_ENABLE(C)                 rcc_periph_clock_enable(C);

struct dev_rng {
	struct device *dev;
	uint32_t base;
	uint32_t *random;
};

#define MAX_RNGS 1

uint32_t req;

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

	uint32_t error_bits = 0;
	error_bits = RNG_SR_SEIS | RNG_SR_CEIS | RNG_SR_SECS | RNG_SR_CECS;

	if (((RNG_SR & error_bits) != 0) ||
		    ((RNG_SR & RNG_SR_DRDY) != 1)) {
		rng->random = (uint32_t *)buf;
		rng_enable_interrupt();
        	rng->dev->pid = scheduler_get_cur_pid();
        	mutex_unlock(rng->dev->mutex);
        	task_suspend();
        	return SYS_CALL_AGAIN;
	}

	rng_get_random((uint32_t *) buf);

	mutex_unlock(rng->dev->mutex);
	return 4;
}


void rng_isr(void)
{
	static int pool = 0;
	struct dev_rng *rng = &DEV_RNG[0];
	uint32_t error_bits = 0;
	error_bits = RNG_SR_SEIS | RNG_SR_CEIS | RNG_SR_SECS | RNG_SR_CECS;
	if ((RNG_SR & RNG_SR_SEIS) != 0) {
		if ((RNG_SR & RNG_SR_DRDY) == 1) {
			uint32_t dummy;
			rng_get_random(&dummy);
		}
		rng_disable();
		rng_enable();
		RNG_SR &= ~RNG_SR_SEIS;
	}
	if ((RNG_SR & RNG_SR_CEIS) != 0) {
		rcc_periph_reset_pulse(RST_RNG);
		rng_disable();
		rng_enable();
		RNG_SR &= ~RNG_SR_CEIS;
	}
	if (((RNG_SR & error_bits) == 0) && ((RNG_SR & RNG_SR_DRDY) == 1)) {
		uint32_t random;
		rng_get_random(&random);
		frand_accu(0, pool, (uint8_t *)&random, 4);
		pool++;
		if (pool >= FRAND_POOL_COUNT) {
			pool = 0;
		}
		if (req == 0) {
			rng_disable_interrupt();
		} else {
			req--;
		}
		//rng_disable_interrupt();
		//task_resume(rng->dev->pid);
	}
}


static void rng_fno_init(struct fnode *dev, uint32_t n, const struct rng_addr *addr)
{
	struct dev_rng *r = &DEV_RNG[n];
	r->dev = device_fno_init(&mod_devrng, mod_devrng.name, dev, FL_RDONLY, r);
	r->base = addr->base;
}

static const struct frand_ops rng_frandops = {  };

static struct frand_info rng_info = { .frandops = (struct frand_ops *)&rng_frandops };

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
	register_frand(&rng_info);
	req = FRAND_ENCRYPT_KEY_SIZE * (FRAND_POOL_COUNT * (SHA256_DIGEST_SIZE / sizeof(word32)) * (SHA256_BLOCK_SIZE / sizeof(word32)));
	rng_enable_interrupt();
}


/* DRIVER INIT */
void stm32_rng_init(void)
{
    register_frand(&rng_info);
    rng_enable_interrupt();
}


