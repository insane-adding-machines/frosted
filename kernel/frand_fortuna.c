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
#include "frand_misc.h"
#include "frand_aes.h"
#include "frand_sha256.h"
#include "frand_types.h"
#include "frand_fortuna.h"
#include <stdint.h>
#include <sys/ioctl.h>

/* Internal helper functions */
static int frand_extract_seed(uint8_t *seed_buffer, int buffer_size);
static int frand_reseed(uint8_t *seed, uint8_t seed_size);
static int frand_generate_block(uint8_t *buffer, int buffer_size);
struct frand_generator_state frand_generator;

struct frand_generator_state frand_generator = {};

/* Sets up the generator */
int frand_fortuna_init(void)
{
	int i = 0;

	frand_generator.key = kalloc(sizeof(uint8_t) * FRAND_ENCRYPT_KEY_SIZE);
	frand_generator.pool = kalloc(sizeof(sha256_t) * FRAND_POOL_COUNT); /* n pools of strings; in practice, hashes iteratively updated */
	frand_generator.counter = kalloc(sizeof(struct frand_counter_fortuna));

	frand_generator.aes = kalloc(sizeof(aes_t));
	frand_generator.sha = kalloc(sizeof(sha256_t));

	if (NULL == frand_generator.key  ||
		NULL == frand_generator.pool ||
		NULL == frand_generator.aes  ||
		NULL == frand_generator.sha) {
		return -1; /* Failed to allocate memory! */
	}

	/* Set counter to 0 (also indicates unseeded state) */
	frand_init_counter(frand_generator.counter);

	for (i = 0; i < FRAND_POOL_COUNT; i++) {
		frand_sha256_init(&(frand_generator.pool[i])); /* TODO right? What does this look like in assembly? */
	}

	return 0;
}

/* Add some entropy to a given pool TODO why need source IDs? */
void frand_accu(int source, int pool, uint8_t *data, int data_size)
{
	/* To be safe, but should avoid needing to do this, as it'll favour lower pools... */
	pool = pool % FRAND_POOL_COUNT;

	/* Add the new entropy data to the specified pool */
	frand_sha256_update(&(frand_generator.pool[pool]), data, data_size);
}

/* Prepare a seed from the selected pools. Helper function for re-seeding. */
static int frand_extract_seed(uint8_t *seed_buffer, int buffer_size)
{
	int i = 0;
	int hashes_added = 0;

	for (i = 0; i < FRAND_POOL_COUNT; i++) {
		if (((i % 8) == 0) || (frand_generator.counter->values[0] & (1 << ((i - 1) % 8)))) {
			/* Use nth pool every (2^n)th reseed (so p1 every other time, p2 every 4th... */
			if ((hashes_added + 1) * FRAND_HASH_SIZE <= buffer_size) {
				/* Extract final hash for given pool, and put in appropriate part of seed buffer */
				frand_sha256_final(&(frand_generator.pool[i]), seed_buffer + (FRAND_HASH_SIZE * hashes_added));
				hashes_added++;
			} else {
				/* Buffer not big enough */
				return 0;
			}
		}
	}

	return (hashes_added * FRAND_HASH_SIZE); /* Actual size of seed */
}

/* Re-seed the generator. Called when enough data has been used from the current 'run' */
static int frand_reseed(uint8_t *seed, uint8_t seed_size)
{
	uint8_t *sha_input = NULL;

	sha_input = (uint8_t *)kalloc(seed_size + FRAND_ENCRYPT_KEY_SIZE); /* Seed size + 256b of current SHA hash key */

	if (NULL == sha_input) {
		/* Failed to alloc, don't increment counter */
		return 0;
	} else {
		memcpy(sha_input, frand_generator.key, FRAND_ENCRYPT_KEY_SIZE); /* Current key */
		memcpy(sha_input + FRAND_ENCRYPT_KEY_SIZE, seed, seed_size); /* New seed */
	}

	frand_sha256_init(frand_generator.sha);
	frand_sha256_update(frand_generator.sha, sha_input, seed_size + 32);
	frand_sha256_final(frand_generator.sha, frand_generator.key);

	frand_increment_counter(frand_generator.counter);

	kfree(sha_input);

	return 1;
}

/* Generate blocks of random data. Underlying function used by the user-callable random data functions. */
static int frand_generate_block(uint8_t *buffer, int buffer_size)
{
	uint8_t encrypt_iv[FRAND_ENCRYPT_IV_SIZE] = { 0 }; /* Can't use ECB mode and combine it with our external counter, so doing it this way */

	/* Run encryption block */
	if (!frand_counter_is_zero(frand_generator.counter)) { /* Refuse if not seeded */
		if (buffer_size >= FRAND_ENCRYPT_BLOCK_SIZE) {
			frand_aes_set_key(frand_generator.aes, frand_generator.key, FRAND_ENCRYPT_KEY_SIZE, encrypt_iv, AES_ENCRYPTION);
			frand_aes_cbc_encrypt(frand_generator.aes, buffer, (const byte *)frand_generator.counter, FRAND_ENCRYPT_IV_SIZE);

			frand_increment_counter(frand_generator.counter);

			return 1; /* Done succesfully */
		}

		/* Buffer size smaller than block size! */
		return 0;
	} else {
		/* Not seeded, cannot produce anything! */
		return 0;
	}
}

/* Get some random bytes from the generator. */
int frand_get_bytes(uint8_t *buffer, int count)
{
	int remaining_bytes = 0;
	uint8_t *block_buffer = 0;
	uint8_t *seed_buffer = 0;
	int blocks_done = 0;
	int seed_size = 0;

	if ((count > 0) && (count < (2 * 1024 * 1024))) {
		// probably we need to initialize and update frand_generator.last_reseed_time?
		if ((jiffies - frand_generator.last_reseed_time >= FRAND_MINIMUM_RESEED_MS) &&
			(1 >= FRAND_MINIMUM_RESEED_ENTR)) { /* FIXME to check 'size' of pool 0 */
			seed_buffer = kalloc(sizeof(uint8_t) * FRAND_POOL_COUNT * FRAND_HASH_SIZE); /* Assume that we use every pool hash... */

			if (NULL == seed_buffer) {
				return 0;
			}

			seed_size = frand_extract_seed(seed_buffer, (sizeof(uint8_t) * FRAND_POOL_COUNT * FRAND_HASH_SIZE));
			frand_reseed(seed_buffer, seed_size);
			kfree(seed_buffer);
		}

		/* Get random blocks until we have our data. */
		for (remaining_bytes = count; remaining_bytes > 0; remaining_bytes -= FRAND_ENCRYPT_BLOCK_SIZE) {
			if (remaining_bytes / FRAND_ENCRYPT_BLOCK_SIZE > 0) { /* At least one full block remaining? Can copy directly without overflowing. */
				frand_generate_block(buffer + (FRAND_ENCRYPT_BLOCK_SIZE * blocks_done++), FRAND_ENCRYPT_BLOCK_SIZE); /* TODO check! */
			} else {
				/* This'll only be necessary for the last block, and only if requested byte count != multiple of block size */
				block_buffer = kalloc(FRAND_ENCRYPT_BLOCK_SIZE);

				if (NULL != block_buffer) {
					frand_generate_block(block_buffer, FRAND_ENCRYPT_BLOCK_SIZE);

					/* Copy required part of block to output */
					memcpy(buffer + (blocks_done * FRAND_ENCRYPT_BLOCK_SIZE),
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

void frand_shutdown(void)
{
	/* Don't just PICO_FREE them, otherwise generator internals still available in RAM! */
	/* If we're going to set it to something, might as well be 10101010, in case there're any weird electrical effects making it easy to detect was-1s or was-0s or something */
	memset(frand_generator.key, 0x55, (sizeof(uint8_t) * 32));
	memset(frand_generator.counter, 0x55, (sizeof(struct frand_counter_fortuna)));
	memset(frand_generator.pool, 0x55, (sizeof(sha256_t) * FRAND_POOL_COUNT));
	memset(frand_generator.aes, 0x55, (sizeof(aes_t)));
	memset(frand_generator.sha, 0x55, (sizeof(sha256_t)));

	kfree(frand_generator.key);
	kfree(frand_generator.counter);
	kfree(frand_generator.pool);
	kfree(frand_generator.aes);
	kfree(frand_generator.sha);
}
