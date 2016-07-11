#ifndef FRAND_TYPES_H_
#define FRAND_TYPES_H_

#include <stdint.h>

struct frand_counter_fortuna {
    uint8_t values[16]; /* Fortuna requires a 128-bit counter */

};

void frand_init_counter(struct frand_counter_fortuna *counter);
void frand_increment_counter(struct frand_counter_fortuna *counter);
void frand_reset_counter(struct frand_counter_fortuna *counter);
int frand_counter_is_zero(struct frand_counter_fortuna *counter);

#endif /* FRAND_TYPES_H_ */
