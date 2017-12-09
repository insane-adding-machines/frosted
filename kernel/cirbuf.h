#include "frosted.h"

#ifndef CIR_BUF_H
#define CIR_BUF_H

struct cirbuf;

struct cirbuf * cirbuf_create(int size);
/* 0 on success, -1 on fail */
int cirbuf_writebyte(struct cirbuf *cb, uint8_t byte);
/* 0 on success, -1 on fail */
int cirbuf_readbyte(struct cirbuf *cb, uint8_t *byte);
/* len on success, -1 on fail */
int cirbuf_writebytes(struct cirbuf *cb, const uint8_t * bytes, int len);

/* len on success, -1 on fail */
int cirbuf_readbytes(struct cirbuf *cb, void *bytes, int len);
int cirbuf_bytesfree(struct cirbuf *cb);
int cirbuf_bytesinuse(struct cirbuf *cb);

#endif
