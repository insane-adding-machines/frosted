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
 *      Authors: Alexander Wood, brabo
 *
 */

#include "frosted.h"
#include "device.h"
#include "frand.h"
#include "frand_fortuna.h"
#include <stdint.h>
#include <sys/ioctl.h>

#if defined(STM32F2) || defined(STM32F4) || defined(STM32F7)
#   include "unicore-mx/stm32/rng.h"
#endif

#define MAX_FRANDS         (1)

static struct frand_info *frand[MAX_FRANDS] = { 0 };

static int frand_open(const char *path, int flags);
static int frand_read(struct fnode *fno, void *buf, unsigned int len);

static struct module mod_devfrand = {
    .family = FAMILY_FILE,
    .name = "frand",
    .ops.open = frand_open,
    .ops.read = frand_read,
};

static int frand_open(const char *path, int flags)
{
	struct fnode *f = fno_search(path);

	if (!f)
		return -1;

	return device_open(path, flags);
}

static int frand_read(struct fnode *fno, void *buf, unsigned int len)
{
	struct frand_info *frand;

	if (len == 0)
		return len;

	frand = (struct frand_info *)FNO_MOD_PRIV(fno, &mod_devfrand);
	if (!frand)
		return -1;

	// mutex_lock(fb->dev->mutex);

	frand_get_bytes(buf, len);

	//mutex_unlock(fb->dev->mutex);
	return len;
}


static int frand_fno_init(struct fnode *dev, struct frand_info *frand)
{
	static int num_frand = 0;
	char name[7] = "frand";

	if (!frand)
		return -1;

	name[6] =  '0' + num_frand++;

	frand->dev = device_fno_init(&mod_devfrand, name, dev, FL_TTY, frand);

	return 0;
}

void frand_init(struct fnode *dev)
{
	/* Ony one FRAND supported for now */
	frand_fno_init(dev, frand[0]);
}

/* Register a low-level frand driver */
int register_frand(struct frand_info *frand_info)
{
	if (!frand_info)
		return -1;

	if (!frand_info->frandops)
		return -1;

	if (frand_info->frandops->frand_open)
		frand_info->frandops->frand_open(frand_info);

	frand[0] = frand_info;

	register_module(&mod_devfrand);
	frand_fortuna_init();

	return 0;
}

