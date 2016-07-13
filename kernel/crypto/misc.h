#ifndef CRYPTO_MISC_H_
#define CRYPTO_MISC_H_

#define XMEMCPY(d,s,l)    memcpy((d),(s),(l))
#define XMEMSET(b,c,l)    memset((b),(c),(l))
#define XMEMCMP(s1,s2,n)  memcmp((s1),(s2),(n))
#define XMEMMOVE(d,s,l)   memmove((d),(s),(l))

#ifndef byte
    typedef unsigned char  byte;
#endif

typedef unsigned short word16;
typedef unsigned int   word32;

word32 rotlFixed(word32 x, word32 y);
word32 rotrFixed(word32 x, word32 y);
word32 ByteReverseWord32(word32 value);
void ByteReverseWords(word32 *out, const word32 *in, word32 byte_count);
void xorbuf(void *buf, const void *mask, word32 count);

#endif /* CRYPTO_MISC_H_ */
