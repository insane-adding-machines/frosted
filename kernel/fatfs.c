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
 *      Based on Petit FatFs R0.03 by ChaN
 *      Copyright (C) 2014, ChaN
 *      When redistribute the Petit FatFs with any modification,
 *      the license can also be changed to GNU GPL or BSD-style license.
 *
 *      Copyright (C) 2016, Insane Adding Machines
 *
 */

#include <stdint.h>
#include "string.h"
#include "frosted.h"

static struct module mod_fatfs = { };

struct fatfs_disk {
    struct fnode *blockdev;
    struct fnode *mountpoint;
    struct fatfs *fs;
};

struct fatfs_priv {
    uint32_t    fptr;
    uint32_t    fsize;      /* File size */
    uint8_t     flag;
    uint32_t dirsect;
    int cluster;
    struct fatfs_disk *fsd;
    uint8_t *dir;
};

typedef uint32_t fatfs_cluster;
#ifdef CONFIG_FAT32
# define FATFS_FAT32	1	/* Enable FAT32 */
#endif
#ifdef CONFIG_FAT16
# define FATFS_FAT16	1	/* Enable FAT16 */
#endif

#ifndef FATFS_FAT16
# define FATFS_FAT16 0
#endif

#ifndef FATFS_FAT32
# define FATFS_FAT32 0
#endif

# define FATFS_FAT32_ONLY 0
#define FATFS_FAT12	1

#define	LD_WORD(ptr)		(uint16_t)(*(uint16_t *)(ptr))
#define	LD_DWORD(ptr)		(uint32_t)(*(uint32_t *)(ptr))
#define FATFS_CODE_PAGE (858)

#define _USE_LCC	1	/* Allow lower case characters for path name */


/* Macro proxies for disk operations */
#define disk_readp(f,b,s,o,l) f->blockdev->owner->ops.block_read(f->blockdev,b,s,o,l)
#define disk_writep(f,b,s,o,l) f->blockdev->owner->ops.block_write(f->blockdev,b,s,o,l)

#define _MAX_SS 512
/* File system object structure */

struct fatfs {
    uint8_t	fs_type;	/* FAT sub type */
    uint8_t	flag;		/* File status flags */
    uint8_t	csize;		/* Number of sectors per cluster */
    uint8_t	pad1;
    uint16_t	n_rootdir;	/* Number of root directory entries (0 on FAT32) */
    fatfs_cluster n_fatent;	/* Number of FAT entries (= number of clusters + 2) */
    uint32_t	fatbase;	/* FAT start sector */
    uint32_t	dirbase;	/* Root directory start sector (Cluster# on FAT32) */
    uint32_t	database;	/* Data start sector */
    fatfs_cluster	org_clust;	/* File start cluster */
    fatfs_cluster	curr_clust;	/* File current cluster */
    uint32_t	dsect;		/* File current data sector */
    uint32_t    fsize; /* FAT size, not file size! */
    uint8_t wflag;
    uint32_t winsect;
    uint8_t win[_MAX_SS];
    uint8_t n_fats;
    uint16_t ssize;
    fatfs_cluster last_clst;
    fatfs_cluster free_clst;
};



/* Directory object structure */

struct fatfs_dir {
    uint16_t	index;		/* Current read/write index number */
    uint8_t*	fn;			/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
    fatfs_cluster	sclust;		/* Table start cluster (0:Static table) */
    fatfs_cluster	clust;		/* Current cluster */
    uint32_t	sect;		/* Current sector */
    uint32_t dptr;
    uint8_t *dir;
};



/* File status structure */

struct fatfs_finfo{
    uint32_t	fsize;		/* File size */
    uint16_t	fdate;		/* Last modified date */
    uint16_t	ftime;		/* Last modified time */
    uint8_t	fattrib;	/* Attribute */
    char	fname[13];	/* File name */
    uint32_t dirsect;
};



/* File function return code (int) */

#define FR_OK            0
#define FR_DISK_ERR      1
#define FR_NOT_READY     2
#define FR_NO_FILE       3
#define FR_NOT_OPENED    4
#define FR_NOT_ENABLED   5
#define FR_NO_FILESYSTEM 6
#define FR_DENIED       7
#define FR_INT_ERR       8

/* File status flag (struct fatfs.flag) */

#define	FA_OPENED	0x01
#define	FA_WPRT		0x02
#define	FA__WIP		0x40

/* DISK status */
#define STA_OK          0x00
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */

/* FAT sub type (struct fatfs.fs_type) */

#define FS_FAT12	1
#define FS_FAT16	2
#define FS_FAT32	3


/* File attribute bits for directory entry */

#define	AM_RDO	0x01	/* Read only */
#define	AM_HID	0x02	/* Hidden */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */
#define AM_MASK	0x3F	/* Mask of defined bits */


/*--------------------------------------------------------------------------

  Module Private Definitions

  ---------------------------------------------------------------------------*/

#define ABORT(err)	{fs->flag = 0; return err;}



/*---------------------------------------------------------------------------/
  / Locale and Namespace Configurations
  /---------------------------------------------------------------------------*/





/*--------------------------------------------------------*/
/* DBCS code ranges and SBCS extend char conversion table */
/*--------------------------------------------------------*/

#if FATFS_CODE_PAGE == 932	/* Japanese Shift-JIS */
#define _DF1S	0x81	/* DBC 1st byte range 1 start */
#define _DF1E	0x9F	/* DBC 1st byte range 1 end */
#define _DF2S	0xE0	/* DBC 1st byte range 2 start */
#define _DF2E	0xFC	/* DBC 1st byte range 2 end */
#define _DS1S	0x40	/* DBC 2nd byte range 1 start */
#define _DS1E	0x7E	/* DBC 2nd byte range 1 end */
#define _DS2S	0x80	/* DBC 2nd byte range 2 start */
#define _DS2E	0xFC	/* DBC 2nd byte range 2 end */

#elif FATFS_CODE_PAGE == 936	/* Simplified Chinese GBK */
#define _DF1S	0x81
#define _DF1E	0xFE
#define _DS1S	0x40
#define _DS1E	0x7E
#define _DS2S	0x80
#define _DS2E	0xFE

#elif FATFS_CODE_PAGE == 949	/* Korean */
#define _DF1S	0x81
#define _DF1E	0xFE
#define _DS1S	0x41
#define _DS1E	0x5A
#define _DS2S	0x61
#define _DS2E	0x7A
#define _DS3S	0x81
#define _DS3E	0xFE

#elif FATFS_CODE_PAGE == 950	/* Traditional Chinese Big5 */
#define _DF1S	0x81
#define _DF1E	0xFE
#define _DS1S	0x40
#define _DS1E	0x7E
#define _DS2S	0xA1
#define _DS2E	0xFE

#elif FATFS_CODE_PAGE == 437	/* U.S. (OEM) */
#define _EXCVT {0x80,0x9A,0x90,0x41,0x8E,0x41,0x8F,0x80,0x45,0x45,0x45,0x49,0x49,0x49,0x8E,0x8F,0x90,0x92,0x92,0x4F,0x99,0x4F,0x55,0x55,0x59,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
    0x41,0x49,0x4F,0x55,0xA5,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 720	/* Arabic (OEM) */
#define _EXCVT {0x80,0x81,0x45,0x41,0x84,0x41,0x86,0x43,0x45,0x45,0x45,0x49,0x49,0x8D,0x8E,0x8F,0x90,0x92,0x92,0x93,0x94,0x95,0x49,0x49,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 737	/* Greek (OEM) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x92,0x92,0x93,0x94,0x95,0x96,0x97,0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87, \
    0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0xAA,0x92,0x93,0x94,0x95,0x96,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0x97,0xEA,0xEB,0xEC,0xE4,0xED,0xEE,0xE7,0xE8,0xF1,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 775	/* Baltic (OEM) */
#define _EXCVT {0x80,0x9A,0x91,0xA0,0x8E,0x95,0x8F,0x80,0xAD,0xED,0x8A,0x8A,0xA1,0x8D,0x8E,0x8F,0x90,0x92,0x92,0xE2,0x99,0x95,0x96,0x97,0x97,0x99,0x9A,0x9D,0x9C,0x9D,0x9E,0x9F, \
    0xA0,0xA1,0xE0,0xA3,0xA3,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xB5,0xB6,0xB7,0xB8,0xBD,0xBE,0xC6,0xC7,0xA5,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xE0,0xE1,0xE2,0xE3,0xE5,0xE5,0xE6,0xE3,0xE8,0xE8,0xEA,0xEA,0xEE,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 850	/* Multilingual Latin 1 (OEM) */
#define _EXCVT {0x80,0x9A,0x90,0xB6,0x8E,0xB7,0x8F,0x80,0xD2,0xD3,0xD4,0xD8,0xD7,0xDE,0x8E,0x8F,0x90,0x92,0x92,0xE2,0x99,0xE3,0xEA,0xEB,0x59,0x99,0x9A,0x9D,0x9C,0x9D,0x9E,0x9F, \
    0xB5,0xD6,0xE0,0xE9,0xA5,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC7,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xE0,0xE1,0xE2,0xE3,0xE5,0xE5,0xE6,0xE7,0xE7,0xE9,0xEA,0xEB,0xED,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 852	/* Latin 2 (OEM) */
#define _EXCVT {0x80,0x9A,0x90,0xB6,0x8E,0xDE,0x8F,0x80,0x9D,0xD3,0x8A,0x8A,0xD7,0x8D,0x8E,0x8F,0x90,0x91,0x91,0xE2,0x99,0x95,0x95,0x97,0x97,0x99,0x9A,0x9B,0x9B,0x9D,0x9E,0x9F, \
    0xB5,0xD6,0xE0,0xE9,0xA4,0xA4,0xA6,0xA6,0xA8,0xA8,0xAA,0x8D,0xAC,0xB8,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBD,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC6,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD1,0xD1,0xD2,0xD3,0xD2,0xD5,0xD6,0xD7,0xB7,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xE0,0xE1,0xE2,0xE3,0xE3,0xD5,0xE6,0xE6,0xE8,0xE9,0xE8,0xEB,0xED,0xED,0xDD,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xEB,0xFC,0xFC,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 855	/* Cyrillic (OEM) */
#define _EXCVT {0x81,0x81,0x83,0x83,0x85,0x85,0x87,0x87,0x89,0x89,0x8B,0x8B,0x8D,0x8D,0x8F,0x8F,0x91,0x91,0x93,0x93,0x95,0x95,0x97,0x97,0x99,0x99,0x9B,0x9B,0x9D,0x9D,0x9F,0x9F, \
    0xA1,0xA1,0xA3,0xA3,0xA5,0xA5,0xA7,0xA7,0xA9,0xA9,0xAB,0xAB,0xAD,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB6,0xB6,0xB8,0xB8,0xB9,0xBA,0xBB,0xBC,0xBE,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC7,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD1,0xD1,0xD3,0xD3,0xD5,0xD5,0xD7,0xD7,0xDD,0xD9,0xDA,0xDB,0xDC,0xDD,0xE0,0xDF, \
    0xE0,0xE2,0xE2,0xE4,0xE4,0xE6,0xE6,0xE8,0xE8,0xEA,0xEA,0xEC,0xEC,0xEE,0xEE,0xEF,0xF0,0xF2,0xF2,0xF4,0xF4,0xF6,0xF6,0xF8,0xF8,0xFA,0xFA,0xFC,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 857	/* Turkish (OEM) */
#define _EXCVT {0x80,0x9A,0x90,0xB6,0x8E,0xB7,0x8F,0x80,0xD2,0xD3,0xD4,0xD8,0xD7,0x98,0x8E,0x8F,0x90,0x92,0x92,0xE2,0x99,0xE3,0xEA,0xEB,0x98,0x99,0x9A,0x9D,0x9C,0x9D,0x9E,0x9E, \
    0xB5,0xD6,0xE0,0xE9,0xA5,0xA5,0xA6,0xA6,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC7,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xE0,0xE1,0xE2,0xE3,0xE5,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xDE,0x59,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 858	/* Multilingual Latin 1 + Euro (OEM) */
#define _EXCVT {0x80,0x9A,0x90,0xB6,0x8E,0xB7,0x8F,0x80,0xD2,0xD3,0xD4,0xD8,0xD7,0xDE,0x8E,0x8F,0x90,0x92,0x92,0xE2,0x99,0xE3,0xEA,0xEB,0x59,0x99,0x9A,0x9D,0x9C,0x9D,0x9E,0x9F, \
    0xB5,0xD6,0xE0,0xE9,0xA5,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC7,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD1,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xE0,0xE1,0xE2,0xE3,0xE5,0xE5,0xE6,0xE7,0xE7,0xE9,0xEA,0xEB,0xED,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 862	/* Hebrew (OEM) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
    0x41,0x49,0x4F,0x55,0xA5,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 866	/* Russian (OEM) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0x90,0x91,0x92,0x93,0x9d,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,0xF0,0xF0,0xF2,0xF2,0xF4,0xF4,0xF6,0xF6,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 874	/* Thai (OEM, Windows) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 1250 /* Central Europe (Windows) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x8A,0x9B,0x8C,0x8D,0x8E,0x8F, \
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xA3,0xB4,0xB5,0xB6,0xB7,0xB8,0xA5,0xAA,0xBB,0xBC,0xBD,0xBC,0xAF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xFF}

#elif FATFS_CODE_PAGE == 1251 /* Cyrillic (Windows) */
#define _EXCVT {0x80,0x81,0x82,0x82,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x80,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x8A,0x9B,0x8C,0x8D,0x8E,0x8F, \
    0xA0,0xA2,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB2,0xA5,0xB5,0xB6,0xB7,0xA8,0xB9,0xAA,0xBB,0xA3,0xBD,0xBD,0xAF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF}

#elif FATFS_CODE_PAGE == 1252 /* Latin 1 (Windows) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0xAd,0x9B,0x8C,0x9D,0xAE,0x9F, \
    0xA0,0x21,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0x9F}

#elif FATFS_CODE_PAGE == 1253 /* Greek (Windows) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xA2,0xB8,0xB9,0xBA, \
    0xE0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xF2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xFB,0xBC,0xFD,0xBF,0xFF}

#elif FATFS_CODE_PAGE == 1254 /* Turkish (Windows) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x8A,0x9B,0x8C,0x9D,0x9E,0x9F, \
    0xA0,0x21,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0x9F}

#elif FATFS_CODE_PAGE == 1255 /* Hebrew (Windows) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
    0xA0,0x21,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 1256 /* Arabic (Windows) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x8C,0x9D,0x9E,0x9F, \
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0x41,0xE1,0x41,0xE3,0xE4,0xE5,0xE6,0x43,0x45,0x45,0x45,0x45,0xEC,0xED,0x49,0x49,0xF0,0xF1,0xF2,0xF3,0x4F,0xF5,0xF6,0xF7,0xF8,0x55,0xFA,0x55,0x55,0xFD,0xFE,0xFF}

#elif FATFS_CODE_PAGE == 1257 /* Baltic (Windows) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xA8,0xB9,0xAA,0xBB,0xBC,0xBD,0xBE,0xAF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xFF}

#elif FATFS_CODE_PAGE == 1258 /* Vietnam (OEM, Windows) */
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0xAC,0x9D,0x9E,0x9F, \
    0xA0,0x21,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xEC,0xCD,0xCE,0xCF,0xD0,0xD1,0xF2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xFE,0x9F}

#else
#error Unknown code page.

#endif



/* Character code support macros */

#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))

#ifndef _EXCVT	/* DBCS configuration */

#ifdef _DF2S	/* Two 1st byte areas */
#define IsDBCS1(c)	(((uint8_t)(c) >= _DF1S && (uint8_t)(c) <= _DF1E) || ((uint8_t)(c) >= _DF2S && (uint8_t)(c) <= _DF2E))
#else			/* One 1st byte area */
#define IsDBCS1(c)	((uint8_t)(c) >= _DF1S && (uint8_t)(c) <= _DF1E)
#endif

#ifdef _DS3S	/* Three 2nd byte areas */
#define IsDBCS2(c)	(((uint8_t)(c) >= _DS1S && (uint8_t)(c) <= _DS1E) || ((uint8_t)(c) >= _DS2S && (uint8_t)(c) <= _DS2E) || ((uint8_t)(c) >= _DS3S && (uint8_t)(c) <= _DS3E))
#else			/* Two 2nd byte areas */
#define IsDBCS2(c)	(((uint8_t)(c) >= _DS1S && (uint8_t)(c) <= _DS1E) || ((uint8_t)(c) >= _DS2S && (uint8_t)(c) <= _DS2E))
#endif

#else			/* SBCS configuration */

#define IsDBCS1(c)	0
#define IsDBCS2(c)	0

#endif /* _EXCVT */

#define SS(fs)  ((fs)->ssize)   /* Variable sector size */

/* FatFs refers the members in the FAT structures with byte offset instead
   / of structure member because there are incompatibility of the packing option
   / between various compilers. */

#define BS_JmpBoot			0
#define BS_OEMName			3
#define BPB_BytsPerSec		11
#define BPB_SecPerClus		13
#define BPB_RsvdSecCnt		14
#define BPB_NumFATs			16
#define BPB_RootEntCnt		17
#define BPB_TotSec16		19
#define BPB_Media			21
#define BPB_FATSz16			22
#define BPB_SecPerTrk		24
#define BPB_NumHeads		26
#define BPB_HiddSec			28
#define BPB_TotSec32		32
#define BS_55AA				510

#define BS_DrvNum			36
#define BS_BootSig			38
#define BS_VolID			39
#define BS_VolLab			43
#define BS_FilSysType		54

#define BPB_FATSz32			36
#define BPB_ExtFlags		40
#define BPB_FSVer			42
#define BPB_RootClus		44
#define BPB_FSInfo			48
#define BPB_BkBootSec		50
#define BS_DrvNum32			64
#define BS_BootSig32		66
#define BS_VolID32			67
#define BS_VolLab32			71
#define BS_FilSysType32		82

#define MBR_Table			446

#define	DIR_Name			0
#define	DIR_Attr			11
#define	DIR_NTres			12
#define	DIR_CrtTime			14
#define	DIR_CrtDate			16
#define	DIR_FstClusHI		20
#define	DIR_WrtTime			22
#define	DIR_WrtDate			24
#define	DIR_FstClusLO		26
#define	DIR_FileSize		28
#define MBR_Table           446     /* MBR: Offset of partition table in the MBR */


#define SZDIRE              32      /* Size of a directory entry */
#define DDEM                0xE5    /* Deleted directory entry mark set to DIR_Name[0] */

#define MAX_FAT12   0xFF5           /* Max FAT12 clusters (differs from specs, but correct for real DOS/Windows behavior) */
#define MAX_FAT16   0xFFF5          /* Max FAT16 clusters (differs from specs, but correct for real DOS/Windows behavior) */
#define MAX_FAT32   0x0FFFFFF5      /* Max FAT32 clusters (not specified, practical limit) */


/*-----------------------------------------------------------------------*/
/* Move/Flush disk access window in the file system object               */
/*-----------------------------------------------------------------------*/
static int sync_window(struct fatfs_disk *f)
{
    struct fatfs *fs = f->fs;
    uint32_t wsect;
    unsigned int nf;
    int res = FR_OK;

    if (fs->wflag) {    /* Write back the sector if it is dirty */
        if (disk_writep(f, fs->win, fs->winsect, 0, SS(fs)) == 0) {
            fs->wflag = 0;
            if (fs->winsect - fs->fatbase < fs->fsize) {      /* Is it in the FAT area? */
                if (fs->n_fats == 2) disk_writep(f, fs->win, fs->winsect + fs->fsize, 0, SS(fs));
            } else {
                res = FR_DISK_ERR;
            }
        }
    }

    return res;
}

static int move_window(struct fatfs_disk *f, uint32_t sector)
{
    struct fatfs *fs = f->fs;
    int res = FR_OK;

    if (sector != fs->winsect) {    /* Window offset changed? */
        res = sync_window(f);      /* Write-back changes */
        if (res == FR_OK) {         /* Fill sector window with new data */
            if (disk_readp(f, fs->win, sector, 0, SS(fs)) != 0) {
                sector = 0xFFFFFFFF;    /* Invalidate window if data is not reliable */
                res = FR_DISK_ERR;
            }
            fs->winsect = sector;
        }
    }
    return res;
}

/*-----------------------------------------------------------------------*/
/* FAT access - Read value of a FAT entry                                */
/*-----------------------------------------------------------------------*/
    /* 1:IO error, Else:Cluster status */

static fatfs_cluster get_fat(struct fatfs_disk *f, fatfs_cluster clst)
{
    struct fatfs *fs = f->fs;
    unsigned int wc, bc;

    if (clst < 2 || clst >= fs->n_fatent) { /* Range check */
        return 1;
    }

    switch (fs->fs_type) {
#if FATFS_FAT12
        case FS_FAT12 :
            bc = (unsigned int)clst;
            bc += bc / 2;
            if (move_window(f, fs->fatbase + (bc / SS(fs))) != FR_OK) break;
            wc = fs->win[bc++ % SS(fs)];
            if (move_window(f, fs->fatbase + (bc / SS(fs))) != FR_OK) break;
            wc |= fs->win[bc % SS(fs)] << 8;
            return (clst & 1) ? (wc >> 4) : (wc & 0xFFF);
#endif

#if FATFS_FAT16
        case FS_FAT16 :
            if (move_window(f, fs->fatbase + (clst / (SS(fs) / 2)))) break;
            return LD_WORD(fs->win + clst * 2 % SS(fs));
#endif
#if FATFS_FAT32
        case FS_FAT32 :
            if (move_window(f, fs->fatbase + (clst / (SS(fs) / 4)))) break;
            return LD_DWORD(fs->win + clst * 4 % SS(fs)) & 0x0FFFFFFF;
#endif
    }

    return 1;
}

static void st_word(uint8_t *ptr, uint16_t val)  /* Store a 2-byte word in little-endian */
{
    *ptr++ = (uint8_t)val; val >>= 8;
    *ptr++ = (uint8_t)val;
}

static void st_dword(uint8_t *ptr, uint32_t val)    /* Store a 4-byte word in little-endian */
{
    *ptr++ = (uint8_t)val; val >>= 8;
    *ptr++ = (uint8_t)val; val >>= 8;
    *ptr++ = (uint8_t)val; val >>= 8;
    *ptr++ = (uint8_t)val;
}

/*-----------------------------------------------------------------------*/
/* FAT access - Change value of a FAT entry                              */
/*-----------------------------------------------------------------------*/

static int put_fat(struct fatfs_disk *f, fatfs_cluster clst, uint32_t val)
{
    struct fatfs *fs = f->fs;
    unsigned int bc;
    uint8_t *p;
    int res = FR_INT_ERR;

    if (clst >= 2 && clst < fs->n_fatent) { /* Check if in valid range */
        switch (fs->fs_type) {
        case FS_FAT12 : /* Bitfield items */
            bc = (unsigned int)clst; bc += bc / 2;
            res = move_window(f, fs->fatbase + (bc / SS(fs)));
            if (res != FR_OK) break;
            p = fs->win + bc++ % SS(fs);
            *p = (clst & 1) ? ((*p & 0x0F) | ((uint8_t)val << 4)) : (uint8_t)val;
            fs->wflag = 1;
            res = move_window(f, fs->fatbase + (bc / SS(fs)));
            if (res != FR_OK) break;
            p = fs->win + bc % SS(fs);
            *p = (clst & 1) ? (uint8_t)(val >> 4) : ((*p & 0xF0) | ((uint8_t)(val >> 8) & 0x0F));
            fs->wflag = 1;
            break;

        case FS_FAT16 : /* WORD aligned items */
            res = move_window(f, fs->fatbase + (clst / (SS(fs) / 2)));
            if (res != FR_OK) break;
            st_word(fs->win + clst * 2 % SS(fs), (uint8_t)val);
            fs->wflag = 1;
            break;

        case FS_FAT32 : /* DWORD aligned items */
            res = move_window(f, fs->fatbase + (clst / (SS(fs) / 4)));
            if (res != FR_OK) break;
            val = (val & 0x0FFFFFFF) | (LD_DWORD(fs->win + clst * 4 % SS(fs)) & 0xF0000000);
            st_dword(fs->win + clst * 4 % SS(fs), val);
            fs->wflag = 1;
            break;
        }
    }
    return res;
}

/*-----------------------------------------------------------------------*/
/* FAT handling - Stretch a chain or Create a new chain                  */
/*-----------------------------------------------------------------------*/
static uint32_t create_chain(struct fatfs_disk *f, fatfs_cluster clst)
{
    struct fatfs *fs = f->fs;
    uint32_t cs, ncl, scl;
    int res;

    if (clst == 0) {    /* Create a new chain */
        scl = fs->last_clst;                /* Get suggested cluster to start from */
        if (scl == 0 || scl >= fs->n_fatent) scl = 1;
    } else {              /* Stretch current chain */
        cs = get_fat(f, clst);            /* Check the cluster status */
        if (cs < 2) return 1;               /* Invalid value */
        if (cs == 0xFFFFFFFF) return cs;    /* A disk error occurred */
        if (cs < fs->n_fatent) return cs;   /* It is already followed by next cluster */
        scl = clst;
    }

    /* On the FAT12/16/32 volume */
    ncl = scl;  /* Start cluster */
    for (;;) {
        ncl++;                          /* Next cluster */
        if (ncl >= fs->n_fatent) {      /* Check wrap-around */
            ncl = 2;
            if (ncl > scl) return 0;    /* No free cluster */
        }
        cs = get_fat(f, ncl);         /* Get the cluster status */
        if (cs == 0) break;             /* Found a free cluster */
        if (cs == 1 || cs == 0xFFFFFFFF) return cs; /* An error occurred */
        if (ncl == scl) return 0;       /* No free cluster */
    }

    res = put_fat(f, ncl, 0xFFFFFFFF); /* Mark the new cluster 'EOC' */
    if ((res == FR_OK) && clst) {
        res = put_fat(f, clst, ncl);   /* Link it from the previous one if needed */
    }

    if (res == FR_OK) {         /* Update FSINFO if function succeeded. */
        fs->last_clst = ncl;
        if (fs->free_clst < fs->n_fatent - 2) fs->free_clst--;
        //fs->fsi_flag |= 1 /* we need the fsi later to indicate action to sync_fs */
    } else {
        ncl = (!res) ? 0xFFFFFFFF : 1;    /* Failed. Create error status */
    }

    return ncl;     /* Return new cluster number or error status */
}


/*-----------------------------------------------------------------------*/
/* Get sector# from cluster# / Get cluster field from directory entry    */
/*-----------------------------------------------------------------------*/
/* !=0: Sector number, 0: Failed - invalid cluster# */

static uint32_t clust2sect (struct fatfs_disk *f, fatfs_cluster clst)
{
    struct fatfs *fs = f->fs;
    clst -= 2;
    if (clst >= (fs->n_fatent - 2)) return 0;		/* Invalid cluster# */
    return (uint32_t)clst * fs->csize + fs->database;
}


    static
fatfs_cluster get_clust ( struct fatfs_disk *f,
        uint8_t* dir		/* Pointer to directory entry */
        )
{
    struct fatfs *fs = f->fs;
    fatfs_cluster clst = 0;


    if (FATFS_FAT32_ONLY || (FATFS_FAT32 && fs->fs_type == FS_FAT32)) {
        clst = LD_WORD(dir+DIR_FstClusHI);
        clst <<= 16;
    }
    clst |= LD_WORD(dir+DIR_FstClusLO);

    return clst;
}


/*-----------------------------------------------------------------------*/
/* Directory handling - Rewind directory index                           */
/*-----------------------------------------------------------------------*/

static int dir_rewind(struct fatfs_disk *f, struct fatfs_dir *dj)
{
	uint32_t clst;
	struct fatfs *fs = f->fs;

	dj->index = 0;
    dj->dptr = 0;
	clst = dj->sclust;
	if (clst == 1 || clst >= fs->n_fatent)	/* Check start cluster range */
		return FR_DISK_ERR;
	if (FATFS_FAT32 && !clst && (FATFS_FAT32_ONLY || fs->fs_type == FS_FAT32))	/* Replace cluster# 0 with root cluster# if in FAT32 */
		clst = (uint32_t)fs->dirbase;
	dj->clust = clst;						/* Current cluster */
	dj->sect = (FATFS_FAT32_ONLY || clst) ? clust2sect(f, clst) : fs->dirbase;	/* Current sector */
    dj->dir = fs->win;

	return FR_OK;	/* Seek succeeded */
}


/*-----------------------------------------------------------------------*/
/* Directory handling - Move directory index next                        */
/*-----------------------------------------------------------------------*/
static int dir_next(struct fatfs_disk *f, struct fatfs_dir *dj, int stretch)
{
    struct fatfs *fs = f->fs;
    uint32_t ofs, clst;
    unsigned int n;

    ofs = dj->dptr + SZDIRE;    /* Next entry */

    if (ofs % SS(fs) == 0) {    /* Sector changed? */
        dj->sect++;             /* Next sector */

        if (!dj->clust) {       /* Static table */
            if (ofs / SZDIRE >= fs->n_rootdir) {    /* Report EOT if it reached end of static table */
                dj->sect = 0; return FR_NO_FILE;
            }
        }
        else {                  /* Dynamic table */
            if ((ofs / SS(fs) & (fs->csize - 1)) == 0) {        /* Cluster changed? */
                clst = get_fat(f, dj->clust);            /* Get next cluster */
                if (clst <= 1) return FR_INT_ERR;               /* Internal error */
                if (clst == 0xFFFFFFFF) return FR_DISK_ERR;     /* Disk error */
                if (clst >= fs->n_fatent) {                     /* Reached end of dynamic table */
                    if (!stretch) {                             /* If no stretch, report EOT */
                        dj->sect = 0; return FR_NO_FILE;
                    }
                    clst = create_chain(f, dj->clust);   /* Allocate a cluster */
                    if (clst == 0) return FR_DENIED;            /* No free cluster */
                    if (clst == 1) return FR_INT_ERR;           /* Internal error */
                    if (clst == 0xFFFFFFFF) return FR_DISK_ERR; /* Disk error */
                    /* Clean-up the stretched table */
                    if (sync_window(f) != FR_OK) return FR_DISK_ERR;   /* Flush disk access window */
                    memset(fs->win, 0, SS(fs));                /* Clear window buffer */
                    for (n = 0, fs->winsect = clust2sect(f, clst); n < fs->csize; n++, fs->winsect++) {    /* Fill the new cluster with 0 */
                        fs->wflag = 1;
                        if (sync_window(f) != FR_OK) return FR_DISK_ERR;
                    }
                    fs->winsect -= n;                           /* Restore window offset */
                }
                dj->clust = clst;       /* Initialize data for new cluster */
                dj->sect = clust2sect(f, clst);
            }
        }
    }
    dj->dptr = ofs;                     /* Current entry */
    dj->dir = fs->win + ofs % SS(fs);   /* Pointer to the entry in the win[] */

    return FR_OK;
}



/*-----------------------------------------------------------------------*/
/* Directory handling - Find an object in the directory                  */
/*-----------------------------------------------------------------------*/
static int dir_find(struct fatfs_disk *f, struct fatfs_dir *dj)
{
    int res;
    uint8_t c;

    res = dir_rewind(f, dj);           /* Rewind directory object */
    if (res != FR_OK) return res;
    /* On the FAT/FAT32 volume */
    do {
        res = move_window(f, dj->sect);
        if (res != FR_OK) break;
        c = dj->dir[DIR_Name];
        if (c == 0) { res = FR_NO_FILE; break; }    /* Reached to end of table */
        //dj->obj.attr = dj->dir[DIR_Attr] & AM_MASK;
        if (!(dj->dir[DIR_Attr] & AM_VOL) && !memcmp(dj->dir, dj->fn, 11)) break;  /* Is it a valid entry? */
        res = dir_next(f, dj, 0);  /* Next entry */
    } while (res == FR_OK);

    return res;
}



/*-----------------------------------------------------------------------*/
/* Read an object from the directory                                     */
/*-----------------------------------------------------------------------*/
static int dir_read(struct fatfs_disk *f, struct fatfs_dir *dj)
{
    int res = FR_NO_FILE;
    uint8_t a, c;

    while (dj->sect) {
        res = move_window(f, dj->sect);
        if (res != FR_OK) break;
        c = dj->dir[DIR_Name];  /* Test for the entry type */
        if (c == 0) {
            res = FR_NO_FILE; break; /* Reached to end of the directory */
        }
        a = dj->dir[DIR_Attr] & AM_MASK; /* Get attribute */
        if (c != DDEM && c != '.' && a != AM_LFN) {    /* Is it a valid entry? */
            break;
        }
        res = dir_next(f, dj, 0);      /* Next entry */
        if (res != FR_OK) break;
    }

    if (res != FR_OK) dj->sect = 0;     /* Terminate the read operation on error or EOT */
    return res;
}

/*-----------------------------------------------------------------------*/
/* Directory handling - Reserve a block of directory entries             */
/*-----------------------------------------------------------------------*/

static int dir_alloc(struct fatfs_disk *f, struct fatfs_dir *dj, unsigned int nent)
{
    int res;
    unsigned int n;

    // res = dir_sdi(f, dp, 0);
    res = dir_rewind(f, dj);            /* Rewind directory object */
    if (res == FR_OK) {
        n = 0;
        do {
            res = move_window(f, dj->sect);
            if (res != FR_OK) break;
            if (dj->dir[DIR_Name] == DDEM || dj->dir[DIR_Name] == 0) {
                if (++n == nent) break; /* A block of contiguous free entries is found */
            } else {
                n = 0;                  /* Not a blank entry. Restart to search */
            }
            res = dir_next(f, dj, 1);
        } while (res == FR_OK); /* Next entry with table stretch enabled */
    }

    if (res == FR_NO_FILE) res = FR_DENIED; /* No directory entry to allocate */
    return res;
}

/*-----------------------------------------------------------------------*/
/* Register an object to the directory                                   */
/*-----------------------------------------------------------------------*/

static int dir_register(struct fatfs_disk *f, struct fatfs_dir *dj)
{
    int res;
    struct fatfs *fs = f->fs;
    res = dir_alloc(f, dj, 1);     /* Allocate an entry for SFN */

    /* Set SFN entry */
    if (res == FR_OK) {
        res = move_window(f, dj->sect);
        if (res == FR_OK) {
            memset(dj->dir, 0, SZDIRE);    /* Clean the entry */
            memcpy(dj->dir + DIR_Name, dj->fn, 11);    /* Put SFN */
            fs->wflag = 1;
        }
    }

    return res;
}

/*-----------------------------------------------------------------------*/
/* Remove an object from the directory                                   */
/*-----------------------------------------------------------------------*/

static int dir_remove(struct fatfs_disk *f, struct fatfs_dir *dj)
{
    int res;
    struct fatfs *fs = f->fs;

    res = move_window(f, dj->sect);
    if (res == FR_OK) {
        dj->dir[DIR_Name] = DDEM;
        fs->wflag = 1;
    }

    return res;
}

/*-----------------------------------------------------------------------*/
/* Pick a segment and create the object name in directory form           */
/*-----------------------------------------------------------------------*/
static
int create_name (
	struct fatfs_dir *dj,			/* Pointer to the directory object */
	const char **path	/* Pointer to pointer to the segment in the path string */
)
{
	char c, ni, si, i, *sfn;
	const char *p;
#if _USE_LCC
#ifdef _EXCVT
	static const char cvt[] = _EXCVT;
#endif
#endif

	/* Create file name in directory form */
	sfn = dj->fn;
	memset(sfn, ' ', 11);
	si = i = 0; ni = 8;
	p = *path;
	for (;;) {
		c = p[si++];
		if (c <= ' ' || c == '/') break;	/* Break on end of segment */
		if (c == '.' || i >= ni) {
			if (ni != 8 || c != '.') break;
			i = 8; ni = 11;
			continue;
		}
#if _USE_LCC
#ifdef _EXCVT
		if (c >= 0x80)					/* To upper extended char (SBCS) */
			c = cvt[c - 0x80];
#endif
		if (IsDBCS1(c) && i < ni - 1) {	/* DBC 1st byte? */
			char d = p[si++];			/* Get 2nd byte */
			sfn[i++] = c;
			sfn[i++] = d;
		} else
#endif
		{						/* Single byte code */
			if (_USE_LCC && IsLower(c)) c -= 0x20;	/* toupper */
			sfn[i++] = c;
		}
	}
	*path = &p[si];						/* Rerurn pointer to the next segment */

	sfn[11] = (c <= ' ') ? 1 : 0;		/* Set last segment flag if end of path */

	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Get file information from directory entry                             */
/*-----------------------------------------------------------------------*/
static void get_fileinfo(struct fatfs_dir *dj, struct fatfs_finfo *fno)
{
	char i, c;
	char *p;

	p = fno->fname;
	if (dj->sect) {
		for (i = 0; i < 8; i++) {	/* Copy file name body */
			c = dj->dir[i];
			if (c == ' ') break;
			if (c == 0x05) c = 0xE5;
			*p++ = c;
		}
		if (dj->dir[8] != ' ') {		/* Copy file name extension */
			*p++ = '.';
			for (i = 8; i < 11; i++) {
				c = dj->dir[i];
				if (c == ' ') break;
				*p++ = c;
			}
		}
		fno->fattrib = dj->dir[DIR_Attr];				/* Attribute */
		fno->fsize = LD_DWORD(dj->dir + DIR_FileSize);	/* Size */
		fno->fdate = LD_WORD(dj->dir + DIR_WrtDate);		/* Date */
		fno->ftime = LD_WORD(dj->dir + DIR_WrtTime);		/* Time */
        fno->dirsect = dj->sect;
	}
	*p = 0;
}



/*-----------------------------------------------------------------------*/
/* Follow a file path                                                    */
/*-----------------------------------------------------------------------*/

static int follow_path(struct fatfs_disk *f, struct fatfs_dir *dj, const char *path)
{
	int res;

	while (*path == ' ') path++;		/* Strip leading spaces */
	if (*path == '/') path++;			/* Strip heading separator if exist */
	dj->sclust = 0;						/* Set start directory (always root dir) */

	if (*path < ' ') {			        /* Null path means the root directory */
		res = dir_rewind(f, dj);
		dj->dir[0] = 0;
	} else {							/* Follow path */
		for (;;) {
			res = create_name(dj, &path);	/* Get a segment */
			if (res != FR_OK) break;
			res = dir_find(f, dj);		/* Find it */
			if (res != FR_OK) break;		/* Could not find the object */
			if (dj->fn[11]) break;			/* Last segment match. Function completed. */
			if (!(dj->dir[DIR_Attr] & AM_DIR)) { /* Cannot follow path because it is a file */
				res = FR_NO_FILE; break;
			}
			dj->sclust = get_clust(f, dj->dir);	/* Follow next */
		}
	}

	return res;
}

char *relative_path(struct fatfs_disk *f, char *abs)
{
    if (!abs)
        return NULL;
    return (abs + strlen(f->mountpoint->fname) + 1);
}


void fatfs_populate(struct fatfs_disk *f, char *path, uint32_t clust)
{
    uint8_t fbuf[12];
    struct fatfs_dir dj;
    struct fnode *parent;
    char fpath[128];
    int res;

    fno_fullpath(f->mountpoint, fpath, 128);
    if (path && strlen(path) > 0) {
        if (path[0] != '/')
            strcat(fpath, "/");
        strcat(fpath, path);
    }
    parent = fno_search(fpath);
    dj.fn = fbuf;
    if (clust > 0) {
        dj.clust = clust;
        dj.sclust = clust;
        res = 0;
    } else {
        res = follow_path(f, &dj, path);
    }

    if (res == 0) {
        dir_rewind(f, &dj);
        while(dir_read(f, &dj) == 0) {
            struct fatfs_finfo fi;
            get_fileinfo(&dj, &fi);
            if (dj.dir[DIR_Attr] & AM_DIR) {
                char fullpath[128];
                strncpy(fullpath, fpath, 128);
                struct fnode *newdir;
                newdir = fno_mkdir(&mod_fatfs, fi.fname, parent);
                strcat(fullpath, "/");
                strcat(fullpath, fi.fname);
                if (newdir) {
                    path = relative_path(f, fullpath);
                    fatfs_populate(f, path, get_clust(f, dj.dir));
                }
            } else {
                struct fnode *newfile;
                newfile = fno_create(&mod_fatfs, fi.fname, parent);
                if (newfile) {
                    newfile->priv = (void *)kalloc(sizeof(struct fatfs_priv));
                    if (!newfile->priv) {
                        fno_unlink(newfile);
                    } else {
                        ((struct fatfs_priv *)newfile->priv)->cluster = get_clust(f, dj.dir);
                        ((struct fatfs_priv *)newfile->priv)->fsd = f;
                        ((struct fatfs_priv *)newfile->priv)->fptr = 0;
                        ((struct fatfs_priv *)newfile->priv)->dirsect = f->fs->winsect;
                        ((struct fatfs_priv *)newfile->priv)->dir = dj.dir;
                        newfile->size = fi.fsize;

                    }
                }
            }
            dir_next(f, &dj, 0);
        }
    }
}

/*-----------------------------------------------------------------------*/
/* Check a sector if it is an FAT boot record                            */
/*-----------------------------------------------------------------------*/


static int check_fs(struct fatfs_disk *f, uint32_t sect)
{
    if (!f || !f->blockdev || !f->blockdev->owner->ops.block_read) {
        return -1;
    }

    struct fatfs *fs = f->fs;

    fs->wflag = 0; fs->winsect = 0xFFFFFFFF;        /* Invaidate window */
    if (move_window(f, sect) != FR_OK) return 4;   /* Load boot record */

    if (LD_WORD((fs->win + BS_55AA)) != 0xAA55)             /* Check record signature */
        return 3;

    if (fs->win[BS_JmpBoot] == 0xE9 || (fs->win[BS_JmpBoot] == 0xEB && fs->win[BS_JmpBoot + 2] == 0x90)) {
        if ((LD_WORD(fs->win + BS_FilSysType)) == 0x4146) return 0;   /* Check "FAT" string */
        if (LD_WORD(fs->win + BS_FilSysType32) == 0x4146) return 0;            /* Check "FAT3" string */
    }

    return 2;
}

static int fatfs_mount(char *source, char *tgt, uint32_t flags, void *arg)
{
    struct fnode *tgt_dir = NULL;
    struct fnode *src_dev = NULL;
    uint8_t fmt, buf[36];
    uint32_t bsect, fsize, tsect, sysect, nclst;
    uint16_t nrsv;
    struct fatfs_disk *fsd;

    /* Source must NOT be NULL */
    if (!source)
        return -1;

    /* Target must be a valid dir */
    if (!tgt)
        return -1;

    tgt_dir = fno_search(tgt);
    src_dev = fno_search(source);

    if (!tgt_dir || ((tgt_dir->flags & FL_DIR) == 0)) {
        /* Not a valid mountpoint. */
        return -1;
    }

    if (!src_dev || !(src_dev ->owner)
            //|| ((src_dev->flags & FL_BLK) == 0)
            ) {
        /* Invalid block device. */
        return -1;
    }

    /* Initialize file system to disk association */
    fsd = kcalloc(sizeof(struct fatfs_disk), 1);
    if (!fsd)
        return -1;

    /* Associate the disk device */
    fsd->blockdev = src_dev;

    /* Associate a newly created fat filesystem */
    fsd->fs = kcalloc(sizeof(struct fatfs), 1);
    if (!fsd->fs) {
        kfree(fsd);
        return -1;
    }

    /* Associate the mount point */
    fsd->mountpoint = tgt_dir;
    tgt_dir->owner = &mod_fatfs;


    fsd->fs->wflag = 0;
    fsd->fs->ssize = 512;
    /* Search FAT partition on the drive */
    bsect = 0;
    fmt = check_fs(fsd, bsect);			/* Check sector 0 as an SFD format */
    struct fatfs *fs = fsd->fs;

    if (fmt == 2) {                     /* Not an FAT boot record, it may be FDISK format */
        bsect = LD_DWORD(fs->win + MBR_Table + 8);
        fmt = check_fs(fsd, bsect);
    }
    if (fmt != 0)
        goto fail;

    fs->last_clst = fs->free_clst = 0xFFFFFFFF;     /* Initialize cluster allocation information */
    fsize = LD_WORD(fs->win + BPB_FATSz16);        /* Number of sectors per FAT */
    if (fsize == 0) fsize = LD_DWORD(fs->win + BPB_FATSz32);
    fs->fsize = fsize;

    fs->n_fats = fs->win[BPB_NumFATs];              /* Number of FATs */
    if (fs->n_fats != 1 && fs->n_fats != 2) goto fail;    /* (Must be 1 or 2) */
    fsize *= fs->n_fats;                           /* Number of sectors for FAT area */

    fs->csize = fs->win[BPB_SecPerClus];            /* Cluster size */
    if (fs->csize == 0 || (fs->csize & (fs->csize - 1))) goto fail;   /* (Must be power of 2) */

    fs->n_rootdir = LD_WORD(fs->win + BPB_RootEntCnt);  /* Number of root directory entries */
    if (fs->n_rootdir % (SS(fs) / SZDIRE)) goto fail; /* (Must be sector aligned) */

    tsect = LD_WORD(fs->win + BPB_TotSec16);        /* Number of sectors on the volume */
    if (tsect == 0) tsect = LD_DWORD(fs->win + BPB_TotSec32);

    nrsv = LD_WORD(fs->win + BPB_RsvdSecCnt);       /* Number of reserved sectors */
    if (nrsv == 0) goto fail;         /* (Must not be 0) */

    /* Determine the FAT sub type */
    sysect = nrsv + fsize + fs->n_rootdir / (SS(fs) / SZDIRE); /* RSV + FAT + DIR */
    if (tsect < sysect) goto fail;    /* (Invalid volume size) */
    nclst = (tsect - sysect) / fs->csize;           /* Number of clusters */
    if (nclst == 0) goto fail;        /* (Invalid volume size) */
    fmt = 0;
    if (nclst <= MAX_FAT32) fmt = FS_FAT32;
    if (nclst <= MAX_FAT16) fmt = FS_FAT16;
    if (nclst <= MAX_FAT12) fmt = FS_FAT12;
    if (fmt == 0) goto fail;

    /* Boundaries and Limits */
    fs->n_fatent = nclst + 2;                       /* Number of FAT entries */
    fs->fatbase = bsect + nrsv;                     /* FAT start sector */
    fs->database = bsect + sysect;                  /* Data start sector */

    if (FATFS_FAT32_ONLY || (FATFS_FAT32 && fmt == FS_FAT32))
        fs->dirbase = LD_DWORD(fs->win + BPB_RootClus); /* Root directory start cluster */
    else
        fs->dirbase = fs->fatbase + fsize;                /* Root directory start sector (lba) */

    fs->fs_type = fmt;
    fs->flag = 0;

    fatfs_populate(fsd, "", 0);
    return 0;

fail:
    kfree(fsd->fs);
    kfree(fsd);
    return -1;
}

static int fatfs_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct fatfs_priv *priv;
    uint32_t sect;
    uint32_t r_len = 0;
    uint32_t r_off = 0;


    if (!fno || !fno->priv)
        return -1;
    priv = (struct fatfs_priv *)fno->priv;

    if (len + fno->off > fno->size)
        len = fno->size - fno->off;

    if (len == 0)
        return -1;

    while (r_len < len) {
        int r = len - r_len;
        r_off = fno->off;
        sect = clust2sect(priv->fsd, priv->cluster);
        while (r_off >= 512) {
            sect++;
            r_off -= 512;
        }
        if (r + r_off > 512) {
            r = 512 - r_off;
        }
        if (disk_readp(priv->fsd, buf + r_len, sect, r_off, r) != 0)
            break;
        r_len += r;
        fno->off += r;
    }
    return r_len;
}


static int fatfs_write(struct fnode *fno, const void *buf, unsigned int len)
{
    struct fatfs_priv *priv;
    struct fatfs_dir dj;
    uint32_t sect;
    uint32_t w_len = 0;
    uint32_t w_off = 0;


    if (!fno || !fno->priv)
        return -1;
    priv = (struct fatfs_priv *)fno->priv;

    /* Check fptr wrap-around (file size cannot reach 4GiB on FATxx) */
    if ((uint32_t)(priv->fptr + len) < (uint32_t)priv->fptr) {
        len = (unsigned int)(0xFFFFFFFF - (uint32_t)priv->fptr);
    }



//    if (len + fno->off > fno->size)
//        len = fno->size - fno->off;

    if (len == 0)
        return -1;

    while (w_len < len) {
        int w = len - w_len;
        w_off = fno->off;
        sect = clust2sect(priv->fsd, priv->cluster);
        while (w_off >= 512) {
            sect++;
            w_off -= 512;
        }
        if (w + w_off > 512) {
            w = 512 - w_off;
        }
        if (disk_writep(priv->fsd, buf + w_len, sect, w_off, w) != 0)
            break;
        w_len += w;
        fno->off += w;
    }
    fno->size = w_len;
    move_window(priv->fsd, priv->dirsect);
    st_dword(priv->dir + DIR_FileSize, (uint32_t)w_len);   /* Update file size */
    priv->fsd->fs->wflag = 1;
    sync_window(priv->fsd);
    return w_len;
}


static int fatfs_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    *revents = events;
    return 1;
}

static int fatfs_seek(struct fnode *fno, int off, int whence)
{
    struct fatfs_fnode *mfno;
    int new_off;
    mfno = FNO_MOD_PRIV(fno, &mod_fatfs);
    if (!mfno)
        return -1;
    switch(whence) {
        case SEEK_CUR:
            new_off = fno->off + off;
            break;
        case SEEK_SET:
            new_off = off;
            break;
        case SEEK_END:
            new_off = fno->size + off;
            break;
        default:
            return -1;
    }

    if (new_off < 0)
        new_off = 0;

    if (new_off > fno->size) {
        return -1;
    }
    fno->off = new_off;
    return fno->off;
}

static int fatfs_close(struct fnode *fno)
{
    struct fatfs_fnode *mfno;
    mfno = FNO_MOD_PRIV(fno, &mod_fatfs);
    if (!mfno)
        return -1;
    fno->off = 0;
    return 0;
}


#if 0

/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

static
int fatfs_open (struct fatfs_disk *f,
        const char *path	/* Pointer to the file name */
        )
{
    int res;
    struct fatfs_dir dj;
    uint8_t sp[12], dir[32];
    struct fatfs *fs = f->fs;


    if (!fs) return FR_NOT_ENABLED;		/* Check file system */

    fs->flag = 0;
    dj.fn = sp;
    res = follow_path(&dj, dir, path);	/* Follow the file path */
    if (res != FR_OK) return res;		/* Follow failed */
    if (!dir[0] || (dir[DIR_Attr] & AM_DIR))	/* It is a directory */
        return FR_NO_FILE;

    fs->org_clust = get_clust(f, dir);		/* File start cluster */
    fs->fsize = LD_DWORD(dir+DIR_FileSize);	/* File size */
    fs->fptr = 0;						/* File pointer */
    fs->flag = FA_OPENED;

    return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

static
int fatfs_read ( struct fatfs_disk *f,
        void* buff,		/* Pointer to the read buffer (NULL:Forward data to the stream)*/
        unsigned int btr,		/* Number of bytes to read */
        unsigned int* br		/* Pointer to number of bytes read */
        )
{
    int dr;
    fatfs_cluster clst;
    uint32_t sect, remain;
    unsigned int rcnt;
    uint8_t cs, *rbuff = buff;
    struct fatfs *fs = f->fs;


    *br = 0;
    if (!fs) return FR_NOT_ENABLED;		/* Check file system */
    if (!(fs->flag & FA_OPENED))		/* Check if opened */
        return FR_NOT_OPENED;

    remain = fs->fsize - fs->fptr;
    if (btr > remain) btr = (unsigned int)remain;			/* Truncate btr by remaining bytes */

    while (btr)	{									/* Repeat until all data transferred */
        if ((fs->fptr % 512) == 0) {				/* On the sector boundary? */
            cs = (uint8_t)(fs->fptr / 512 & (fs->csize - 1));	/* Sector offset in the cluster */
            if (!cs) {								/* On the cluster boundary? */
                if (fs->fptr == 0)					/* On the top of the file? */
                    clst = fs->org_clust;
                else
                    clst = get_fat(fs->curr_clust);
                if (clst <= 1) ABORT(FR_DISK_ERR);
                fs->curr_clust = clst;				/* Update current cluster */
            }
            sect = clust2sect(f, fs->curr_clust);		/* Get current sector */
            if (!sect) ABORT(FR_DISK_ERR);
            fs->dsect = sect + cs;
        }
        rcnt = 512 - (unsigned int)fs->fptr % 512;			/* Get partial sector data from sector buffer */
        if (rcnt > btr) rcnt = btr;
        dr = disk_readp(!buff ? 0 : rbuff, fs->dsect, (unsigned int)fs->fptr % 512, rcnt);
        if (dr) ABORT(FR_DISK_ERR);
        fs->fptr += rcnt; rbuff += rcnt;			/* Update pointers and counters */
        btr -= rcnt; *br += rcnt;
    }

    return FR_OK;
}



/*-----------------------------------------------------------------------*/
/* Write File                                                            */
/*-----------------------------------------------------------------------*/
static
int fatfs_write (struct fatfs_disk *f,
        const void* buff,	/* Pointer to the data to be written */
        unsigned int btw,			/* Number of bytes to write (0:Finalize the current write operation) */
        unsigned int* bw			/* Pointer to number of bytes written */
        )
{
    fatfs_cluster clst;
    uint32_t sect, remain;
    const uint8_t *p = buff;
    uint8_t cs;
    unsigned int wcnt;
    struct fatfs *fs = f->fs;


    *bw = 0;
    if (!fs) return FR_NOT_ENABLED;		/* Check file system */
    if (!(fs->flag & FA_OPENED))		/* Check if opened */
        return FR_NOT_OPENED;

    if (!btw) {		/* Finalize request */
        if ((fs->flag & FA__WIP) && disk_writep(0, 0)) ABORT(FR_DISK_ERR);
        fs->flag &= ~FA__WIP;
        return FR_OK;
    } else {		/* Write data request */
        if (!(fs->flag & FA__WIP))		/* Round-down fptr to the sector boundary */
            fs->fptr &= 0xFFFFFE00;
    }
    remain = fs->fsize - fs->fptr;
    if (btw > remain) btw = (unsigned int)remain;			/* Truncate btw by remaining bytes */

    while (btw)	{									/* Repeat until all data transferred */
        if ((unsigned int)fs->fptr % 512 == 0) {			/* On the sector boundary? */
            cs = (uint8_t)(fs->fptr / 512 & (fs->csize - 1));	/* Sector offset in the cluster */
            if (!cs) {								/* On the cluster boundary? */
                if (fs->fptr == 0)					/* On the top of the file? */
                    clst = fs->org_clust;
                else
                    clst = get_fat(fs->curr_clust);
                if (clst <= 1) ABORT(FR_DISK_ERR);
                fs->curr_clust = clst;				/* Update current cluster */
            }
            sect = clust2sect(f, fs->curr_clust);		/* Get current sector */
            if (!sect) ABORT(FR_DISK_ERR);
            fs->dsect = sect + cs;
            if (disk_writep(0, fs->dsect)) ABORT(FR_DISK_ERR);	/* Initiate a sector write operation */
            fs->flag |= FA__WIP;
        }
        wcnt = 512 - (unsigned int)fs->fptr % 512;			/* Number of bytes to write to the sector */
        if (wcnt > btw) wcnt = btw;
        if (disk_writep(p, wcnt)) ABORT(FR_DISK_ERR);	/* Send data to the sector */
        fs->fptr += wcnt; p += wcnt;				/* Update pointers and counters */
        btw -= wcnt; *bw += wcnt;
        if ((unsigned int)fs->fptr % 512 == 0) {
            if (disk_writep(0, 0)) ABORT(FR_DISK_ERR);	/* Finalize the currtent secter write operation */
            fs->flag &= ~FA__WIP;
        }
    }

    return FR_OK;
}



/*-----------------------------------------------------------------------*/
/* Seek File R/W Pointer                                                 */
/*-----------------------------------------------------------------------*/

static
int fatfs_lseek (struct fatfs_disk *f,
        uint32_t ofs		/* File pointer from top of file */
        )
{
    fatfs_cluster clst;
    uint32_t bcs, sect, ifptr;
    struct fatfs *fs = f->fs;


    if (!fs) return FR_NOT_ENABLED;		/* Check file system */
    if (!(fs->flag & FA_OPENED))		/* Check if opened */
        return FR_NOT_OPENED;

    if (ofs > fs->fsize) ofs = fs->fsize;	/* Clip offset with the file size */
    ifptr = fs->fptr;
    fs->fptr = 0;
    if (ofs > 0) {
        bcs = (uint32_t)fs->csize * 512;	/* Cluster size (byte) */
        if (ifptr > 0 &&
                (ofs - 1) / bcs >= (ifptr - 1) / bcs) {	/* When seek to same or following cluster, */
            fs->fptr = (ifptr - 1) & ~(bcs - 1);	/* start from the current cluster */
            ofs -= fs->fptr;
            clst = fs->curr_clust;
        } else {							/* When seek to back cluster, */
            clst = fs->org_clust;			/* start from the first cluster */
            fs->curr_clust = clst;
        }
        while (ofs > bcs) {				/* Cluster following loop */
            clst = get_fat(clst);		/* Follow cluster chain */
            if (clst <= 1 || clst >= fs->n_fatent) ABORT(FR_DISK_ERR);
            fs->curr_clust = clst;
            fs->fptr += bcs;
            ofs -= bcs;
        }
        fs->fptr += ofs;
        sect = clust2sect(f, clst);		/* Current sector */
        if (!sect) ABORT(FR_DISK_ERR);
        fs->dsect = sect + (fs->fptr / 512 & (fs->csize - 1));
    }

    return FR_OK;
}
#endif

int fatfs_init(void)
{
    mod_fatfs.family = FAMILY_FILE;
    strcpy(mod_fatfs.name,"fatfs");

    mod_fatfs.mount = fatfs_mount;
    mod_fatfs.ops.read = fatfs_read;
    mod_fatfs.ops.write = fatfs_write;
    mod_fatfs.ops.seek = fatfs_seek;
    mod_fatfs.ops.poll = fatfs_poll;

    /*
    mod_fatfs.ops.creat = fatfs_creat;
    mod_fatfs.ops.unlink = fatfs_unlink;
    mod_fatfs.ops.close = fatfs_close;
    mod_fatfs.ops.exe = fatfs_exe;
    */

    register_module(&mod_fatfs);
    return 0;
}
