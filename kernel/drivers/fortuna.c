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
#include "crypto/misc.h"
#include "crypto/aes.h"
#include "crypto/sha256.h"
#include "fortuna.h"
#include <stdint.h>
#include <sys/ioctl.h>

/* Fortuna architecture defines */
#define FORTUNA_POOL_COUNT            1
#define FORTUNA_MINIMUM_RESEED_MS     100
#define FORTUNA_MINIMUM_RESEED_ENTR   1
#define FORTUNA_MAX_REQUEST_BYTES     1048576

struct fortuna_counter {
    uint8_t values[16]; /* Fortuna requires a 128-bit counter */

};

/* Internal generator state */
struct fortuna_generator_state {
	/* Fortuna internals */
	uint8_t *key; /* 32 byte key */
	struct fortuna_counter *counter;
	Sha256 *pool;
	unsigned int last_reseed_time;

	/* AES and SHA internal stuff */
	Aes *aes;
	Sha256 *sha;
};

struct fortuna_generator_state fortuna_generator = {};

/* Internal helper functions */
static int fortuna_extract_seed(uint8_t *seed_buffer, int buffer_size);
static int fortuna_reseed(uint8_t *seed, uint8_t seed_size);
static int fortuna_generate_block(uint8_t *buffer, int buffer_size);

static inline void fortuna_init_counter(struct fortuna_counter *counter) {
	int i;

	for (i = 0; i < 16; i++) {
		counter->values[i] = 0;

	}

}

static inline void fortuna_increment_counter(struct fortuna_counter *counter) {
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

static int fortuna_counter_is_zero(struct fortuna_counter *counter) {
	int i;

	for (i = 0; i < 16; i++) {
		if (counter->values[i] != 0) {
			return 0;

		}

	}

	return 1; /* None of the individual values are non-zero, so... */

}

/* Prepare a seed from the selected pools. Helper function for re-seeding. */
static int fortuna_extract_seed(uint8_t *seed_buffer, int buffer_size)
{
	int i = 0;
	int hashes_added = 0;

	for (i = 0; i < FORTUNA_POOL_COUNT; i++) {
		if (((i % 8) == 0) || (fortuna_generator.counter->values[0] & (1 << ((i - 1) % 8)))) {
			/* Use nth pool every (2^n)th reseed (so p1 every other time, p2 every 4th... */
			if ((hashes_added + 1) * FORTUNA_HASH_SIZE <= buffer_size) {
				/* Extract final hash for given pool, and put in appropriate part of seed buffer */
				wc_Sha256Final(&(fortuna_generator.pool[i]), seed_buffer + (FORTUNA_HASH_SIZE * hashes_added));
				hashes_added++;
			} else {
				/* Buffer not big enough */
				return 0;
			}
		}
	}

	return (hashes_added * FORTUNA_HASH_SIZE); /* Actual size of seed */
}

/* Re-seed the generator. Called when enough data has been used from the current 'run' */
static int fortuna_reseed(uint8_t *seed, uint8_t seed_size)
{
	uint8_t *sha_input = NULL;

	sha_input = (uint8_t *)kalloc(seed_size + FORTUNA_ENCRYPT_KEY_SIZE); /* Seed size + 256b of current SHA hash key */

	if (NULL == sha_input) {
		/* Failed to alloc, don't increment counter */
		return 0;
	} else {
		memcpy(sha_input, fortuna_generator.key, FORTUNA_ENCRYPT_KEY_SIZE); /* Current key */
		memcpy(sha_input + FORTUNA_ENCRYPT_KEY_SIZE, seed, seed_size); /* New seed */
	}

	wc_InitSha256(fortuna_generator.sha);
	wc_Sha256Update(fortuna_generator.sha, sha_input, seed_size + 32);
	wc_Sha256Final(fortuna_generator.sha, fortuna_generator.key);

	fortuna_increment_counter(fortuna_generator.counter);

	kfree(sha_input);

	return 1;
}

/* Generate blocks of random data. Underlying function used by the user-callable random data functions. */
static int fortuna_generate_block(uint8_t *buffer, int buffer_size)
{
	uint8_t encrypt_iv[FORTUNA_ENCRYPT_IV_SIZE] = { 0 }; /* Can't use ECB mode and combine it with our external counter, so doing it this way */

	/* Run encryption block */
	if (!fortuna_counter_is_zero(fortuna_generator.counter)) { /* Refuse if not seeded */
		if (buffer_size >= FORTUNA_ENCRYPT_BLOCK_SIZE) {
			wc_AesSetKey(fortuna_generator.aes, fortuna_generator.key, FORTUNA_ENCRYPT_KEY_SIZE, encrypt_iv, AES_ENCRYPTION);
			wc_AesCbcEncrypt(fortuna_generator.aes, buffer, (const byte *)fortuna_generator.counter, FORTUNA_ENCRYPT_IV_SIZE);

			fortuna_increment_counter(fortuna_generator.counter);

			return 1; /* Done succesfully */
		}

		/* Buffer size smaller than block size! */
		return 0;
	} else {
		/* Not seeded, cannot produce anything! */
		return 0;
	}
}

/* Public Fortuna functions */
/* Sets up the generator */
int fortuna_init(void)
{
	int i = 0;

	fortuna_generator.key = kalloc(sizeof(uint8_t) * FORTUNA_ENCRYPT_KEY_SIZE);
	fortuna_generator.pool = kalloc(sizeof(Sha256) * FORTUNA_POOL_COUNT); /* n pools of strings; in practice, hashes iteratively updated */
	fortuna_generator.counter = kalloc(sizeof(struct fortuna_counter));

	fortuna_generator.aes = kalloc(sizeof(Aes));
	fortuna_generator.sha = kalloc(sizeof(Sha256));

	if (NULL == fortuna_generator.key  ||
		NULL == fortuna_generator.pool ||
		NULL == fortuna_generator.aes  ||
		NULL == fortuna_generator.sha) {
		return -1; /* Failed to allocate memory! */
	}

	/* Set counter to 0 (also indicates unseeded state) */
	fortuna_init_counter(fortuna_generator.counter);

	for (i = 0; i < FORTUNA_POOL_COUNT; i++) {
		wc_InitSha256(&(fortuna_generator.pool[i])); /* TODO right? What does this look like in assembly? */
	}

	return 0;
}

/* Add some entropy to a given pool TODO why need source IDs? */
void fortuna_accu(int source, int pool, uint8_t *data, int data_size)
{
	/* To be safe, but should avoid needing to do this, as it'll favour lower pools... */
	pool = pool % FORTUNA_POOL_COUNT;

	/* Add the new entropy data to the specified pool */
	wc_Sha256Update(&(fortuna_generator.pool[pool]), data, data_size);
}

/* Get some random bytes from the generator. */
int fortuna_get_bytes(uint8_t *buffer, int count)
{
	int remaining_bytes = 0;
	uint8_t *block_buffer = 0;
	uint8_t *seed_buffer = 0;
	int blocks_done = 0;
	int seed_size = 0;

	if ((count > 0) && (count < (2 * 1024 * 1024))) {
		// probably we need to initialize and update fortuna_generator.last_reseed_time?
		if ((jiffies - fortuna_generator.last_reseed_time >= FORTUNA_MINIMUM_RESEED_MS) &&
			(1 >= FORTUNA_MINIMUM_RESEED_ENTR)) { /* FIXME to check 'size' of pool 0 */
			seed_buffer = kalloc(sizeof(uint8_t) * FORTUNA_POOL_COUNT * FORTUNA_HASH_SIZE); /* Assume that we use every pool hash... */

			if (NULL == seed_buffer) {
				return 0;
			}

			seed_size = fortuna_extract_seed(seed_buffer, (sizeof(uint8_t) * FORTUNA_POOL_COUNT * FORTUNA_HASH_SIZE));
			fortuna_reseed(seed_buffer, seed_size);
			kfree(seed_buffer);
		}

		/* Get random blocks until we have our data. */
		for (remaining_bytes = count; remaining_bytes > 0; remaining_bytes -= FORTUNA_ENCRYPT_BLOCK_SIZE) {
			if (remaining_bytes / FORTUNA_ENCRYPT_BLOCK_SIZE > 0) { /* At least one full block remaining? Can copy directly without overflowing. */
				fortuna_generate_block(buffer + (FORTUNA_ENCRYPT_BLOCK_SIZE * blocks_done++), FORTUNA_ENCRYPT_BLOCK_SIZE); /* TODO check! */
			} else {
				/* This'll only be necessary for the last block, and only if requested byte count != multiple of block size */
				block_buffer = kalloc(FORTUNA_ENCRYPT_BLOCK_SIZE);

				if (NULL != block_buffer) {
					fortuna_generate_block(block_buffer, FORTUNA_ENCRYPT_BLOCK_SIZE);

					/* Copy required part of block to output */
					memcpy(buffer + (blocks_done * FORTUNA_ENCRYPT_BLOCK_SIZE),
					block_buffer, remaining_bytes);

					kfree(block_buffer);

				} else {
					return 0;
				}
			}
		}

		return 1;
	} else {
		/* Not allowed to produce more than 1MiB of data at once */
		return 0;
	}
}

void fortuna_shutdown(void)
{
	/* Don't just PICO_FREE them, otherwise generator internals still available in RAM! */
	/* If we're going to set it to something, might as well be 10101010, in case there're any weird electrical effects making it easy to detect was-1s or was-0s or something */
	memset(fortuna_generator.key, 0x55, (sizeof(uint8_t) * 32));
	memset(fortuna_generator.counter, 0x55, (sizeof(struct fortuna_counter)));
	memset(fortuna_generator.pool, 0x55, (sizeof(Sha256) * FORTUNA_POOL_COUNT));
	memset(fortuna_generator.aes, 0x55, (sizeof(Aes)));
	memset(fortuna_generator.sha, 0x55, (sizeof(Sha256)));

	kfree(fortuna_generator.key);
	kfree(fortuna_generator.counter);
	kfree(fortuna_generator.pool);
	kfree(fortuna_generator.aes);
	kfree(fortuna_generator.sha);
}
