#ifndef CRYPTO_SHA256_H_
#define CRYPTO_SHA256_H_

#include "crypto/misc.h"

/* in bytes */
enum {
	SHA256             =  2,   /* hash type unique */
	SHA256_BLOCK_SIZE  = 64,
	SHA256_DIGEST_SIZE = 32,
	SHA256_PAD_SIZE    = 56
};

/* Sha256 digest */
typedef struct Sha256 {
	word32 buffLen;   /* in bytes          */
	word32 loLen;     /* length in bytes   */
	word32 hiLen;     /* length in bytes   */
	word32 digest[SHA256_DIGEST_SIZE / sizeof(word32)];
	word32 buffer[SHA256_BLOCK_SIZE  / sizeof(word32)];
} Sha256;

int wc_InitSha256(Sha256 *sha256);
int wc_Sha256Update(Sha256 *sha256, const byte *data, word32 len);
int wc_Sha256Final(Sha256 *sha256, byte *hash);

#endif /* CRYPTO_SHA256_H_ */
