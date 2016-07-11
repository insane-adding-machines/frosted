#ifndef FRAND_AES_H_
#define FRAND_AES_H_

enum {
	AES_ENC_TYPE   = 1,   /* cipher unique type */
	AES_ENCRYPTION = 0,
	AES_DECRYPTION = 1,
	AES_BLOCK_SIZE = 16
};

typedef struct aes_t {
	/* AESNI needs key first, rounds 2nd, not sure why yet */
	__attribute__ ( (aligned (16))) word32 key[60];
	word32  rounds;

	__attribute__ ( (aligned (16))) word32 reg[AES_BLOCK_SIZE / sizeof(word32)];      /* for CBC mode */
	__attribute__ ( (aligned (16))) word32 tmp[AES_BLOCK_SIZE / sizeof(word32)];      /* same         */

	void *heap; /* memory hint to use */
} aes_t;

int frand_aes_set_iv(aes_t *aes, const byte *iv);
int frand_aes_set_key(aes_t *aes, const byte *user_key, word32 keylen, const byte *iv,
	int dir);
int frand_aes_cbc_encrypt(aes_t *aes, byte *out, const byte *in, word32 sz);

#endif /* FRAND_AES_H_ */
