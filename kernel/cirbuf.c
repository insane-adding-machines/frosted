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
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  

#include "frosted.h"

struct cirbuf {
    uint8_t *buf;
    uint8_t *readptr;
    uint8_t *writeptr;
    int     bufsize;
};

struct cirbuf * cirbuf_create(int size)
{
    struct cirbuf* inbuf;
    if (size <= 0) 
        return NULL;

    inbuf = kalloc(sizeof(struct cirbuf));
    if (!inbuf)
        return NULL;

    inbuf->buf = kalloc(size);
    if (!inbuf->buf)
    {
        kfree(inbuf);
        return NULL;
    }

    inbuf->bufsize = size;
    inbuf->readptr = inbuf->buf;
    inbuf->writeptr = inbuf->buf;
    return inbuf;
}

/* 0 on success, -1 on fail */
int cirbuf_writebyte(struct cirbuf *cb, uint8_t byte)
{
    if (!cb)
        return -1;

    /* check if there is space */
    if (!cirbuf_bytesfree(cb))
        return -1;

    *cb->writeptr++ = byte;

    /* wrap if needed */
    if (cb->writeptr > cb->buf+cb->bufsize)
        cb->writeptr = cb->buf;

    return 0;
}

/* 0 on success, -1 on fail */
int cirbuf_readbyte(struct cirbuf *cb, uint8_t *byte)
{
    if (!cb || !byte)
        return -1;

    /* check if there is data */
    if (!cirbuf_bytesinuse(cb))
        return -1;

    *byte = *cb->readptr++;

    /* wrap if needed */
    if (cb->readptr > cb->buf+cb->bufsize)
        cb->readptr = cb->buf;

    return 0;
}

/* written len on success, 0 on fail */
int cirbuf_writebytes(struct cirbuf *cb, uint8_t * bytes, int len)
{
    uint8_t byte;
    int freesize;
    if (!cb)
        return 0;

    /* check if there is space */
    freesize = cirbuf_bytesfree(cb);
    if (!freesize)
        return 0;
    if (freesize < len)
        len = freesize;

    /* Wrap needed ? */
    if ((cb->writeptr + len) > (cb->buf + cb->bufsize))
    {
        int len_first_part = cb->buf + cb->bufsize - cb->writeptr; /* end - current position */
        memcpy(cb->writeptr, bytes, len_first_part);
        bytes += len_first_part;
        cb->writeptr = cb->buf; /* set to start of buffer */
        len -= len_first_part;
    }
    /* write remaining part */
    if (len)
    {
        memcpy(cb->writeptr, bytes, len);
        cb->writeptr += len;
    }

    return len;
}

int cirbuf_bytesfree(struct cirbuf *cb)
{
    int bytes;
    if (!cb)
        return -1;

    bytes = (int)(cb->readptr - cb->writeptr - 1);
    if (cb->writeptr >= cb->readptr)
        bytes += cb->bufsize;

    return bytes;
}

int cirbuf_bytesinuse(struct cirbuf *cb)
{
    int bytes;
    if (!cb)
        return -1;

    bytes = (int)(cb->writeptr - cb->readptr);
    if (cb->writeptr < cb->readptr)
        bytes += cb->bufsize;

    return (bytes);
}

