#ifndef FRAND_SHA256_H_
#define FRAND_SHA256_H_

/* in bytes */
enum {
	SHA256             =  2,   /* hash type unique */
	SHA256_BLOCK_SIZE  = 64,
	SHA256_DIGEST_SIZE = 32,
	SHA256_PAD_SIZE    = 56
};

/* Sha256 digest */
typedef struct sha256_t {
	word32 buf_len;   /* in bytes          */
	word32 lo_len;     /* length in bytes   */
	word32 hi_len;     /* length in bytes   */
	word32 digest[SHA256_DIGEST_SIZE / sizeof(word32)];
	word32 buffer[SHA256_BLOCK_SIZE  / sizeof(word32)];
} sha256_t;

int frand_sha256_init(sha256_t *sha256);
int frand_sha256_update(sha256_t *sha256, const byte *data, word32 len);
int frand_sha256_final(sha256_t *sha256, byte *hash);

#endif /* FRAND_SHA256_H_ */
