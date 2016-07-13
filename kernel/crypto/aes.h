#ifndef CRYPTO_AES_H_
#define CRYPTO_AES_H_

#include "crypto/misc.h"

enum {
	AES_ENC_TYPE   = 1,   /* cipher unique type */
	AES_ENCRYPTION = 0,
	AES_DECRYPTION = 1,
	AES_BLOCK_SIZE = 16
};

typedef struct Aes {
	/* AESNI needs key first, rounds 2nd, not sure why yet */
	__attribute__ ( (aligned (16))) word32 key[60];
	word32  rounds;

	__attribute__ ( (aligned (16))) word32 reg[AES_BLOCK_SIZE / sizeof(word32)];      /* for CBC mode */
	__attribute__ ( (aligned (16))) word32 tmp[AES_BLOCK_SIZE / sizeof(word32)];      /* same         */

	void *heap; /* memory hint to use */
} Aes;


int wc_AesSetKey(Aes *aes, const byte *userKey, word32 keylen, const byte *iv,
			int dir);
int wc_AesCbcEncrypt(Aes *aes, byte *out, const byte *in, word32 sz);
int wc_AesCbcDecrypt(Aes *aes, byte *out, const byte *in, word32 sz);

#endif /* CRYPTO_AES_H_ */
