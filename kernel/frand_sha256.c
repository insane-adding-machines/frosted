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
 *      Copyright (C) 2006-2016 wolfSSL Inc.
 *	Code submitted to wolfSSL by raphael.huck@efixo.com
 *
 */

#include "frosted.h"
#include "frand_misc.h"
#include "frand_sha256.h"

int frand_sha256_init(sha256_t *sha256)
{
	int ret = 0;
	sha256->digest[0] = 0x6A09E667L;
	sha256->digest[1] = 0xBB67AE85L;
	sha256->digest[2] = 0x3C6EF372L;
	sha256->digest[3] = 0xA54FF53AL;
 	sha256->digest[4] = 0x510E527FL;
	sha256->digest[5] = 0x9B05688CL;
	sha256->digest[6] = 0x1F83D9ABL;
	sha256->digest[7] = 0x5BE0CD19L;

	sha256->buf_len = 0;
	sha256->lo_len   = 0;
	sha256->hi_len   = 0;

	return ret;
}

#define CH(x,y,z)       ((z) ^ ((x) & ((y) ^ (z))))
#define MAJ(x,y,z)      ((((x) | (y)) & (z)) | ((x) & (y)))
#define R(x, n)         (((x)&0xFFFFFFFFU)>>(n))

#define S(x, n)         rotr_fixed(x, n)
#define SIGMA0(x)       (S(x, 2) ^ S(x, 13) ^ S(x, 22))
#define SIGMA1(x)       (S(x, 6) ^ S(x, 11) ^ S(x, 25))
#define GAMMA0(x)       (S(x, 7) ^ S(x, 18) ^ R(x, 3))
#define GAMMA1(x)       (S(x, 17) ^ S(x, 19) ^ R(x, 10))

#define RND(a,b,c,d,e,f,g,h,i) \
     t0 = (h) + SIGMA1((e)) + CH((e), (f), (g)) + K[(i)] + W[(i)]; \
     t1 = SIGMA0((a)) + MAJ((a), (b), (c)); \
     (d) += t0; \
     (h)  = t0 + t1;

static const __attribute__ ( (aligned (32))) word32 K[64] = {
    0x428A2F98L, 0x71374491L, 0xB5C0FBCFL, 0xE9B5DBA5L, 0x3956C25BL,
    0x59F111F1L, 0x923F82A4L, 0xAB1C5ED5L, 0xD807AA98L, 0x12835B01L,
    0x243185BEL, 0x550C7DC3L, 0x72BE5D74L, 0x80DEB1FEL, 0x9BDC06A7L,
    0xC19BF174L, 0xE49B69C1L, 0xEFBE4786L, 0x0FC19DC6L, 0x240CA1CCL,
    0x2DE92C6FL, 0x4A7484AAL, 0x5CB0A9DCL, 0x76F988DAL, 0x983E5152L,
    0xA831C66DL, 0xB00327C8L, 0xBF597FC7L, 0xC6E00BF3L, 0xD5A79147L,
    0x06CA6351L, 0x14292967L, 0x27B70A85L, 0x2E1B2138L, 0x4D2C6DFCL,
    0x53380D13L, 0x650A7354L, 0x766A0ABBL, 0x81C2C92EL, 0x92722C85L,
    0xA2BFE8A1L, 0xA81A664BL, 0xC24B8B70L, 0xC76C51A3L, 0xD192E819L,
    0xD6990624L, 0xF40E3585L, 0x106AA070L, 0x19A4C116L, 0x1E376C08L,
    0x2748774CL, 0x34B0BCB5L, 0x391C0CB3L, 0x4ED8AA4AL, 0x5B9CCA4FL,
    0x682E6FF3L, 0x748F82EEL, 0x78A5636FL, 0x84C87814L, 0x8CC70208L,
    0x90BEFFFAL, 0xA4506CEBL, 0xBEF9A3F7L, 0xC67178F2L
};

static int transform(sha256_t *sha256)
{
	word32 S[8], t0, t1;
	int i;

	word32 W[64];

	/* Copy context->state[] to working vars */
	for (i = 0; i < 8; i++)
		S[i] = sha256->digest[i];

	for (i = 0; i < 16; i++)
		W[i] = sha256->buffer[i];

	for (i = 16; i < 64; i++)
		W[i] = GAMMA1(W[i-2]) + W[i-7] + GAMMA0(W[i-15]) + W[i-16];

	for (i = 0; i < 64; i += 8) {
		RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],i+0);
		RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],i+1);
		RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],i+2);
		RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],i+3);
		RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],i+4);
		RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],i+5);
		RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],i+6);
		RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],i+7);
	}

	/* Add the working vars back into digest state[] */
	for (i = 0; i < 8; i++) {
		sha256->digest[i] += S[i];
	}

	return 0;
}

#define XTRANSFORM(sha256, B) transform(sha256)

static inline void add_length(sha256_t *sha256, word32 len)
{
    word32 tmp = sha256->lo_len;
    if ((sha256->lo_len += len) < tmp)
        sha256->hi_len++;                       /* carry low to high */
}

int frand_sha256_update(sha256_t *sha256, const byte *data, word32 len)
{
	/* do block size increments */
	byte* local = (byte*)sha256->buffer;

	while (len) {
		word32 add = min(len, SHA256_BLOCK_SIZE - sha256->buf_len);
		XMEMCPY(&local[sha256->buf_len], data, add);

		sha256->buf_len += add;
		data            += add;
		len             -= add;

		if (sha256->buf_len == SHA256_BLOCK_SIZE) {
			int ret;

			byte_rev_words(sha256->buffer, sha256->buffer, SHA256_BLOCK_SIZE);
			ret = XTRANSFORM(sha256, local);
			if (ret != 0)
				return ret;

			add_length(sha256, SHA256_BLOCK_SIZE);
			sha256->buf_len = 0;
		}
	}

	return 0;
}

int frand_sha256_final(sha256_t *sha256, byte *hash)
{
	byte *local = (byte *)sha256->buffer;
	int ret;

	add_length(sha256, sha256->buf_len);  /* before adding pads */

	local[sha256->buf_len++] = 0x80;     /* add 1 */

	/* pad with zeros */
	if (sha256->buf_len > SHA256_PAD_SIZE) {
		XMEMSET(&local[sha256->buf_len], 0, SHA256_BLOCK_SIZE - sha256->buf_len);
		sha256->buf_len += SHA256_BLOCK_SIZE - sha256->buf_len;

		byte_rev_words(sha256->buffer, sha256->buffer, SHA256_BLOCK_SIZE);

		ret = XTRANSFORM(sha256, local);
		if (ret != 0)
			return ret;

		sha256->buf_len = 0;
	}
	XMEMSET(&local[sha256->buf_len], 0, SHA256_PAD_SIZE - sha256->buf_len);

	/* put lengths in bits */
	sha256->hi_len = (sha256->lo_len >> (8*sizeof(sha256->lo_len) - 3)) +
		(sha256->hi_len << 3);
	sha256->lo_len = sha256->lo_len << 3;

	/* store lengths */
	byte_rev_words(sha256->buffer, sha256->buffer, SHA256_BLOCK_SIZE);

	/* ! length ordering dependent on digest endian type ! */
	XMEMCPY(&local[SHA256_PAD_SIZE], &sha256->hi_len, sizeof(word32));
	XMEMCPY(&local[SHA256_PAD_SIZE + sizeof(word32)], &sha256->lo_len,
		sizeof(word32));

        byte_rev_words(&sha256->buffer[SHA256_PAD_SIZE/sizeof(word32)],
        	&sha256->buffer[SHA256_PAD_SIZE/sizeof(word32)],
		2 * sizeof(word32));

	ret = XTRANSFORM(sha256, local);
	if (ret != 0)
		return ret;

	byte_rev_words(sha256->digest, sha256->digest, SHA256_DIGEST_SIZE);
	XMEMCPY(hash, sha256->digest, SHA256_DIGEST_SIZE);

	return frand_sha256_init(sha256);  /* reset state */
}
