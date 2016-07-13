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
 *
 */

#include "frosted.h"
#include "crypto/misc.h"


word32 rotlFixed(word32 x, word32 y)
{
	return (x << y) | (x >> (sizeof(y) * 8 - y));
}

word32 rotrFixed(word32 x, word32 y)
{
	return (x >> y) | (x << (sizeof(y) * 8 - y));
}

word32 ByteReverseWord32(word32 value)
{
	/* 6 instructions with rotate instruction, 8 without */
	value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
	return rotlFixed(value, 16U);
}

void ByteReverseWords(word32 *out, const word32 *in, word32 byte_count)
{
	word32 count = byte_count/(word32)sizeof(word32), i;

	for (i = 0; i < count; i++)
		out[i] = ByteReverseWord32(in[i]);
}

static void XorWords(word32 *r, const word32 *a, word32 n)
{
	word32 i;

	for (i = 0; i < n; i++) r[i] ^= a[i];
}

void xorbuf(void *buf, const void *mask, word32 count)
{
	if ((((word32)buf | (word32)mask | count) % sizeof(word32)) == 0)
		XorWords((word32 *)buf, (const word32 *)mask, (count / sizeof(word32)));
	else {
		word32 i;
		byte *b = (byte *)buf;
		const byte *m = (const byte *)mask;

		for (i = 0; i < count; i++) b[i] ^= m[i];
	}
}
