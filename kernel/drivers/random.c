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
#include "cirbuf.h"
#include "random.h"

#ifdef LM3S
#   include "libopencm3/lm3s/rng.h"
#   define CLOCK_ENABLE(C) 
#endif
#ifdef STM32F4
#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f4/rng.h>
#   define CLOCK_ENABLE(C)                 rcc_periph_clock_enable(C);
#endif
#ifdef LPC17XX
#   include "libopencm3/lpc17xx/rng.h"
#   define CLOCK_ENABLE(C) 
#endif

struct dev_rng {
	struct device * dev;
	uint32_t base;
	//struct cirbuf *inbuf;
	//struct cirbuf *outbuf;
	uint8_t *w_start;
	uint8_t *w_end;
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
	int out;

	static uint32_t last_value;
	static uint32_t new_value;

	uint32_t error_bits = 0;
	error_bits = RNG_SR_SEIS | RNG_SR_CEIS;
	while (new_value == last_value) {
		/* Check for error flags and if data is ready. */
		if (((RNG_SR & error_bits) == 0) &&
		    ((RNG_SR & RNG_SR_DRDY) == 1)) {
			new_value = RNG_DR;
		}
	}
	last_value = new_value;

	*((uint32_t *) buf) = new_value;
	out = 4;

	return out;
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

		/* Enable interupt */
		/* Set the IE bit in the RNG_CR register. */
		RNG_CR |= RNG_CR_IE;
		/* Enable the random number generation by setting the RNGEN bit in
		   the RNG_CR register. This activates the analog part, the RNG_LFSR
		   and the error detector.
		*/
		RNG_CR |= RNG_CR_RNGEN;
	}
	register_module(&mod_devrng);
}

