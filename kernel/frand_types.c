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
 *      Authors: Alexander Wood
 *
 */

#include "frosted.h"
#include "frand_types.h"

inline void frand_init_counter(struct frand_counter_fortuna *counter) {
	int i;

	for (i = 0; i < 16; i++) {
		counter->values[i] = 0;

	}

}

inline void frand_increment_counter(struct frand_counter_fortuna *counter) {
	int i;

	for (i = 0; i < 16; i++) {
		if (counter->values[i] < 0xFF) { /* Not at maximum value */
			counter->values[i]++;
			return; /* Only return once we've found one not ready to overflow */

		} else {
			counter->values[i] = 0;

		}

	}

}

int frand_counter_is_zero(struct frand_counter_fortuna *counter) {
	int i;

	for (i = 0; i < 16; i++) {
		if (counter->values[i] != 0) {
			return 0;

		}

	}

	return 1; /* None of the individual values are non-zero, so... */

}
