#ifndef FORTUNA_H_
#define FORTUNA_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Defines for hash and encryption algorithms */
#define FORTUNA_HASH_SIZE             32
#define FORTUNA_ENCRYPT_IV_SIZE       16
#define FORTUNA_ENCRYPT_KEY_SIZE      32
#define FORTUNA_ENCRYPT_BLOCK_SIZE    16

int fortuna_init(void);
void fortuna_accu(int source, int pool, uint8_t *data, int data_size);

/* Get random stuff! */
int fortuna_get_bytes(uint8_t *buffer, int count);

/* Shut down the generator securely if it is no longer needed */
void fortuna_shutdown(void);

#endif /* FORTUNA_H_ */

