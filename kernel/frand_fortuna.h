#ifndef FRAND_FORTUNA_H_
#define FRAND_FORTUNA_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "frosted.h"
#include "frand_misc.h"
#include "frand_aes.h"
#include "frand_sha256.h"
#include "frand_types.h"

/* Fortuna architecture defines */
#define FRAND_POOL_COUNT            32
#define FRAND_MINIMUM_RESEED_MS     100
#define FRAND_MINIMUM_RESEED_ENTR   1
#define FRAND_MAX_REQUEST_BYTES     1048576

/* Defines for hash and encryption algorithms */
#define FRAND_HASH_SIZE             32
#define FRAND_ENCRYPT_IV_SIZE       16
#define FRAND_ENCRYPT_KEY_SIZE      32
#define FRAND_ENCRYPT_BLOCK_SIZE    16

/* Internal generator state */
struct frand_generator_state {
	/* Fortuna internals */
	uint8_t *key; /* 32 byte key */
	struct frand_counter_fortuna *counter;
	sha256_t *pool;
	unsigned int last_reseed_time;

	/* AES and SHA internal stuff */
	aes_t *aes;
	sha256_t *sha;
};

extern struct frand_generator_state frand_generator;

int frand_fortuna_init(void);
void frand_accu(int source, int pool, uint8_t *data, int data_size);

/* Get random stuff! */
int frand_get_bytes(uint8_t *buffer, int count);
uint32_t frand_fortuna(void);

/* Shut down the generator securely if it is no longer needed */
void frand_fortuna_shutdown(void);

#endif /* FRAND_FORTUNA_H_ */

