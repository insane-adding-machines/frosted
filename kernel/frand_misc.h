#ifndef FRAND_MISC_H_
#define FRAND_MISC_H_

#define XMEMCPY(d,s,l)    memcpy((d),(s),(l))
#define XMEMSET(b,c,l)    memset((b),(c),(l))
#define XMEMCMP(s1,s2,n)  memcmp((s1),(s2),(n))
#define XMEMMOVE(d,s,l)   memmove((d),(s),(l))

#ifndef byte
    typedef unsigned char  byte;
#endif

typedef unsigned short word16;
typedef unsigned int   word32;

word32 min(word32 a, word32 b);
word32 rotl_fixed(word32 x, word32 y);
word32 rotr_fixed(word32 x, word32 y);
word32 byte_rev_word32(word32 value);
void byte_rev_words(word32 *out, const word32 *in, word32 byte_count);
void xor_words(word32 *r, const word32 *a, word32 n);
void xorbuf(void *buf, const void *mask, word32 count);

#endif /* FRAND_MISC_H_ */
