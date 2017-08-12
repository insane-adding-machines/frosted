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
 *      Authors: brabo
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
    uint32_t            sclust;
    uint32_t            cclust;
    uint32_t            sect;
    uint32_t            off;
    uint32_t            dirsect;
    struct fatfs_disk   *fsd;
    uint32_t            *fat;
};

struct fatfs {
    int         fd;
    uint8_t     win[512];
    uint32_t    bsect;
    uint8_t     type;
    uint16_t    bps;
    uint8_t     spc;
    uint32_t    database;
    uint32_t    fatbase;
    uint32_t    dirbase;
    uint32_t    n_fatent;
    uint8_t     mounted;
};

struct fatfs_dir {
    uint8_t     *fn;
    uint32_t    sclust;
    uint32_t    cclust;
    uint32_t    sect;
    uint32_t    off;
    uint32_t    dirsect;
    uint32_t    attr;
    uint32_t    fsize;
};

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

/* Macro proxies for disk operations */
#define disk_read(f,b,s,o,l) f->blockdev->owner->ops.block_read(f->blockdev,b,s,o,l)
#define disk_write(f,b,s,o,l) f->blockdev->owner->ops.block_write(f->blockdev,b,s,o,l)

#define BS_JMPBOOT      0
#define BPB_BPS         11
#define BPB_SPC         13
#define BPB_RSVD_SEC    14
#define BPB_NUMFATS     16
#define BPB_ROOTENTS    17
#define BPB_TOTSEC16    19
#define BPB_FATSz16     22
#define BPB_TOTSEC32    32
#define BPB_FATSz32     36
#define BPB_ROOTCLUS    44
#define BS_FSTYPE       54
#define BS_FSTYPE32     82
#define BS_MBR          446
#define BS_BS           454
#define BS_55AA         510

#define DEFBPS          512

#define DIR_NAME        0
#define DIR_ATTR        11
#define DIR_SCLUST_HI   20
#define DIR_SCLUST_LO   26
#define DIR_FSIZE       28

#define AM_DIR  0x10    /* Directory */

#define FAT12   1
#define FAT16   2
#define FAT32   3
#define EOC32   0x0FFFFFF8

#define LD_WORD(ptr)        (uint16_t)(*(uint16_t *)(ptr))
#define LD_DWORD(ptr)       (uint32_t)(*(uint32_t *)(ptr))

#define CLUST2SECT(f, c) (((c - 2) * f->spc + f->database))

static void st_word(uint8_t *ptr, uint16_t val)  /* Store a 2-byte word in little-endian */
{
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
}

static void st_dword(uint8_t *ptr, uint32_t val)    /* Store a 4-byte word in little-endian */
{
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
}

static int check_fs(struct fatfs_disk *fsd)
{
    struct fatfs *fs = fsd->fs;
    if (LD_WORD(fs->win + BS_55AA) != 0xAA55) {
        return -1;
    }

    if (fs->win[BS_JMPBOOT] == 0xE9 || (fs->win[BS_JMPBOOT] == 0xEB && fs->win[BS_JMPBOOT + 2] == 0x90)) {

        if ((LD_WORD(fs->win + BS_FSTYPE)) == 0x4146) return 0;   /* Check "FAT" string */
        if ((LD_WORD(fs->win + BS_FSTYPE32) == 0x4146) && (LD_WORD(fs->win + BS_FSTYPE32 + 2) == 0x3354) && (*(fs->win + BS_FSTYPE32 + 4) == 0x32)) {
            fs->type = FAT32;
            return 0;            /* Check "FAT3" string */
        }
    }

    return -2;
}

static int get_fat(struct fatfs_disk *fsd, int clust)
{
    struct fatfs *fs = fsd->fs;

    if (clust < 2 || clust >= fs->n_fatent) { /* Range check */
        return -1;
    }

    switch(fs->type) {
    case FAT12:
        break;
    case FAT16:
        if (disk_read(fsd, fs->win, (fs->fatbase + (clust / (fs->bps / 2))), 0, fs->bps) < 0) break;
        return LD_WORD(fs->win + (clust * 2));
    case FAT32:
        if (disk_read(fsd, fs->win, (fs->fatbase + (clust / (fs->bps / 4))), 0, fs->bps) < 0) break;

        return (LD_DWORD(fs->win + ((clust * 4) & (fs->bps - 1))) & 0x0FFFFFFF);
    }
    return -2;
}

static int set_fat(struct fatfs_disk *fsd, uint32_t clust, uint32_t val)
{
    struct fatfs *fs = fsd->fs;

    if (clust < 2 || clust >= fs->n_fatent) { /* Range check */
        return -1;
    }

    switch(fs->type) {
    case FAT12:
        break;
    case FAT16:
        //if (mb_read(fs->fd, fs->win, 0, (fs->fatbase + (clust / (fs->bps / 2))) * fs->bps, fs->bps)) break;
        //return LD_WORD(fs->win + (clust * 2));
        break;
    case FAT32:
        if (disk_read(fsd, fs->win, (fs->fatbase + (clust / (fs->bps / 4))), 0, fs->bps) < 0) break;
        st_dword((fs->win + ((clust * 4) & (fs->bps - 1))), (uint32_t)(val & 0x0FFFFFFF));
        disk_write(fsd, fs->win, (fs->fatbase + (clust / (fs->bps / 4))), 0, fs->bps);
        return 0;
    }
    return -2;
}

static int get_clust(struct fatfs *fs, uint8_t *dir)
{
    int clst = 0;

    if (fs->type == FAT32) {
        clst = LD_WORD(dir + DIR_SCLUST_HI);
        clst <<= 16;
    }
    clst |= LD_WORD(dir + DIR_SCLUST_LO);

    return clst;
}

static int set_clust(struct fatfs *fs, uint8_t *dir, uint32_t clust)
{
    if (fs->type == FAT32) {
        st_word((dir + DIR_SCLUST_HI), (clust >> 16));
    }
    st_word((dir + DIR_SCLUST_LO), (clust & 0xFFFF));

    return 0;
}

static int walk_fat(struct fatfs_disk *fsd)
{
    int fat;
    int clust = 2;

   while (2 > 1) {
        fat = get_fat(fsd, clust);
        if (!fat)
            break;
        clust++;
    }

    return clust;
}

static int init_fat(struct fatfs_disk *fsd)
{
    struct fatfs *fs = fsd->fs;

    int clust = walk_fat(fsd);

    st_dword((fs->win + ((clust * 4) & (fs->bps - 1))), 0x0FFFFFFF);
    disk_write(fsd, fs->win, (fs->fatbase + (clust / (fs->bps / 4))), 0, fs->bps);

    return clust;
}

static int dir_rewind(struct fatfs *fs, struct fatfs_dir *dj)
{
    if (dj->cclust == 1 || dj->cclust >= fs->n_fatent)
        return -1;

    if (!dj->cclust)
        dj->sect = fs->dirbase;
    else
        dj->sect = CLUST2SECT(fs, dj->cclust);

    dj->off = 0;

    return 0;
}

static int dir_read(struct fatfs_disk *fsd, struct fatfs_dir *dj)
{
    /* the dir_read logic needs refactoring.
        for one it sucks we have to -= 32 the offset in populate */
    if (!fsd || !dj)
        return -1;

    struct fatfs *fs = fsd->fs;

    if (dj->off == 512) {
        dj->sect++;
        dj->off = 0;
    }

    disk_read(fsd, fs->win, (dj->sect), 0, fs->bps);

    /* have to check cluster borders! */
    while (2 > 1) {
        uint8_t *off = fs->win + dj->off;
        if (!*off) /* Free FAT entry, no more FAT entries! */
            return -2;

        if (*(off + DIR_ATTR) & 0x0F) { /* LFN entry */
            dj->off += 32;
            continue;
        } else {
            int i;
            char *p = dj->fn;
            memset(dj->fn, 0x00, 13);
            char c;

            for (i = 0; i < 8; i++) {   /* Copy file name body */
                c = off[i];
                if (c == ' ') break;
                if (c == 0x05) c = 0xE5;
                *p++ = c;
            }

            if (off[8] != ' ') {        /* Copy file name extension */
                *p++ = '.';
                for (i = 8; i < 11; i++) {
                    c = off[i];
                    if (c == ' ') break;
                    *p++ = c;
                }
            }

            dj->attr = *(off + DIR_ATTR);
            dj->sclust = get_clust(fs, off);
            dj->fsize = LD_DWORD(off + DIR_FSIZE);
            dj->dirsect = dj->sect;
            dj->off += 32;

            break;
        }
    }

    return 0;
}

static int add_dir(struct fatfs *fs, struct fatfs_dir *dj, char *name)
{
    int nlen = strlen(name);
    if (nlen > 12)
        return -1;

    memset((fs->win + dj->off), 0, 0x20);
    memset((fs->win + dj->off), ' ', 11);

    int len = 0;
    while (len < nlen) {
        *(fs->win + dj->off + len) = name[len++];
    }

    *(fs->win + dj->off + DIR_ATTR) = 0x20;
    st_dword((fs->win + dj->off + DIR_FSIZE), 0);
    dj->dirsect = dj->sect;

    return 0;
}

static char *relative_path(struct fatfs_disk *f, char *abs)
{
    if (!abs)
        return NULL;

    return (abs + strlen(f->mountpoint->fname) + 1);
}

static void fatfs_populate(struct fatfs_disk *f, char *path, uint32_t clust)
{
    char fbuf[13];
    struct fatfs_priv *priv;
    struct fatfs_dir dj;
    struct fnode *parent;
    char fpath[MAXPATHLEN];

    fno_fullpath(f->mountpoint, fpath, MAXPATHLEN);

    if (path && strlen(path) > 0) {
        if (path[0] != '/')
            strcat(fpath, "/");
        strcat(fpath, path);
    }

    parent = fno_search(fpath);
    parent->priv = (void *)kalloc(sizeof(struct fatfs_priv));
    ((struct fatfs_priv *)parent->priv)->fsd = f;

    dj.fn = fbuf;
    dj.cclust = clust;
    dj.sclust = clust;
    dj.off = 0;

    dir_rewind(f->fs, &dj);

    while(dir_read(f, &dj) == 0) {
       if (!strncmp(dj.fn, ".", 2) || !strncmp(dj.fn, "..", 3))
            continue;

        struct fnode *newdir;

        if (dj.attr & AM_DIR)
            newdir = fno_mkdir(&mod_fatfs, dj.fn, parent);
        else
            newdir = fno_create(&mod_fatfs, dj.fn, parent);

        if (!newdir)
            continue;

        newdir->priv = (void *)kalloc(sizeof(struct fatfs_priv));
        if (!newdir->priv) {
            fno_unlink(newdir);
            continue;
        }

        priv = newdir->priv;
        priv->sclust = dj.sclust;
        priv->cclust = dj.cclust;
        priv->sect = dj.sect;
        priv->fsd = f;
        priv->off = dj.off - 32;
        priv->dirsect = dj.dirsect;

        newdir->size = dj.fsize;
        newdir->off = 0;

        int i = 0;
        uint32_t nclust = newdir->size / (f->fs->spc * f->fs->bps);
        priv->fat = kcalloc(sizeof (uint32_t), nclust + 2);
        uint32_t tmpclust = priv->sclust;
        priv->fat[i++] = tmpclust;
        while (nclust > 0) {
            priv->fat[i] = get_fat(f, tmpclust);
            tmpclust = priv->fat[i++];
            nclust--;
        }

        if (dj.attr & AM_DIR) {
            char fullpath[128];
            strncpy(fullpath, fpath, 128);
            strcat(fullpath, "/");
            strcat(fullpath, dj.fn);
            path = relative_path(f, fullpath);
            fatfs_populate(f, path, get_clust(f->fs, (f->fs->win + (dj.off - 32))));
        }
    }
}

int fatfs_mount(char *source, char *tgt, uint32_t flags, void *arg)
{
    struct fnode *tgt_dir = NULL;
    struct fnode *src_dev = NULL;

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

    struct fatfs *fs = fsd->fs;
    /* do we still need this?? */
    fs->mounted = 0;

    /* Associate the mount point */
    fsd->mountpoint = tgt_dir;
    tgt_dir->owner = &mod_fatfs;
    fs->bsect = 0;

    disk_read(fsd, fs->win, fs->bsect, 0, DEFBPS);

    if (check_fs(fsd) == -2) {
        fs->bsect = LD_WORD(fs->win + BS_BS);
        disk_read(fsd, fs->win, fs->bsect, 0, DEFBPS);

        if (check_fs(fsd) < 0) {
            goto fail;
        }
    }

    uint8_t num_fats = 0;
    uint16_t root_ents = 0, rsvd_sec = 0;
    uint32_t root_secs = 0, fatsz = 0, totsec = 0, datasec = 0, nclusts = 0;

    fs->bps = LD_WORD(fs->win + BPB_BPS);
    fs->spc = fs->win[BPB_SPC];

    root_ents = LD_WORD(fs->win + BPB_ROOTENTS);
    root_secs = ((root_ents * 32) + (fs->bps -1)) / fs->bps;

    fatsz = LD_WORD(fs->win + BPB_FATSz16);
    if (!fatsz)
        fatsz = LD_DWORD(fs->win + BPB_FATSz32);

    num_fats = fs->win[BPB_NUMFATS];
    rsvd_sec = LD_WORD(fs->win + BPB_RSVD_SEC);
    fs->database = rsvd_sec + (num_fats * fatsz) + root_secs;
    fs->database += fs->bsect;

    totsec = LD_WORD(fs->win + BPB_TOTSEC16);
    if (!totsec)
        totsec = LD_DWORD(fs->win + BPB_TOTSEC32);

    datasec = totsec - (rsvd_sec + (num_fats * fatsz) + root_secs);
    nclusts = datasec / fs->spc;
    fs->fatbase = rsvd_sec + fs->bsect;
    fs->dirbase = CLUST2SECT(fs, LD_DWORD(fs->win + BPB_ROOTCLUS));

    /* FAT type determination */
    if (nclusts < 4085)
        fs->type = FAT12;
    else if (nclusts < 65525)
        fs->type = FAT16;
    else
        fs->type = FAT32;

    fs->n_fatent = nclusts + 2;

    fatfs_populate(fsd, "", 0);
    fs->mounted = 1;

    return 0;

fail:
    kfree(fsd->fs);
    kfree(fsd);

    return -1;
}

int fatfs_open(char *path, uint32_t flags)
{
    struct fnode *fno;
    fno = fno_search(path);
    if (!fno)
        return -1;

    fno->off = 0;

    return 0;
}

static int dir_find(struct fatfs_disk *fsd, struct fatfs_dir *dj, char *path)
{
    if (!fsd || !dj || !path)
        return -1;

    while (dir_read(fsd, dj) == 0) {
        if (!strncmp(dj->fn, path, 12)) {
            dj->off -= 32;
            uint32_t fat = get_fat(fsd, dj->sclust);
            dj->sect = CLUST2SECT(fsd->fs, dj->sclust);
            dj->cclust = dj->sclust;
            return 0;
        }
    }

    return -2;
}

static int follow_path(struct fatfs_disk *fsd, struct fatfs_dir *dj, char *path)
{
    int res;

    if (!strncmp(path, "/mnt", 4))
        path += 4;

    while (*path == ' ')
        path++;

    if (*path == '/')
        path++;

    dj->cclust = 0;

    res = dir_rewind(fsd->fs, dj);
    if (*path < ' ') {
        dj->off = 0;
        return res;
    }

    dj->off = 0;

    do {
        char tpath[12];
        char *tpathp = tpath;

        while ((*path != '/') && (*path != ' ') && (*path != 0x00)) {
            *tpathp++ = *path++;

        }
        *tpathp = 0x00;
        res = dir_find(fsd, dj, tpath);
        if (*path == '/')
            path++;
    } while ((*path != ' ') && (*path != 0x00));

    return res;
}

int fatfs_create(struct fnode *fno)
{
    if (!fno || !fno->parent || !fno->parent->priv)
        return -1;

    if (!((struct fatfs_priv *)fno->parent->priv)->fsd->fs->mounted)
        return -2;

    struct fatfs_dir dj;
    char fbuf[13];
    char *path = kalloc(MAXPATHLEN * sizeof (char));

    if (!fno || !fno->parent)
        return -1;

    fno_fullpath(fno, path, MAXPATHLEN);

    struct fatfs_priv *priv = (struct fatfs_priv *)fno->priv;

    if (!priv) {
        priv = (void *)kalloc(sizeof (struct fatfs_priv));
        priv->fsd = ((struct fatfs_priv *)fno->parent->priv)->fsd;
    }

    struct fatfs *fs = priv->fsd->fs;
    struct fatfs_disk *fsd = priv->fsd;
    fno->priv = priv;
    dj.fn = fbuf;
    dj.off = 0;

    uint32_t clust = init_fat(fsd);
    follow_path(fsd, &dj, path);

    while ((*path != ' ') && (*path != 0x00))
        path++;
    while (*path != '/')
        path--;
    path++;

    int ret = add_dir(fs, &dj, path);
    set_clust(fs, (fs->win + dj.off), clust);
    disk_write(fsd, fs->win, dj.sect, 0, fs->bps);

    priv->sclust = priv->cclust = clust;
    priv->sect = CLUST2SECT(fs, priv->cclust);
    priv->off = dj.off;
    fno->off = 0;
    fno->size = 0;
    priv->dirsect = dj.dirsect;

    return 0;
}

int fatfs_read(struct fnode *fno, void *buf, unsigned int len)
{
    if (!fno || !fno->priv)
        return -1;

    struct fatfs_priv *priv = (struct fatfs_priv *)fno->priv;
    struct fatfs_disk *fsd = priv->fsd;
    struct fatfs *fs = fsd->fs;

    int r_len = 0, sect = 0, off = 0, clust = 0;

    /* calculate where to put sect and off! */
    off = fno->off;
    sect = off / fs->bps;
    off = off & (fs->bps - 1);
    clust = sect / fs->spc;
    sect = sect & (fs->spc - 1);

    priv->cclust = priv->fat[clust];
    priv->sect = CLUST2SECT(fs, priv->cclust);

    while ((r_len < len) && (fno->off < fno->size)) {
        int r = len - r_len;
        if (r > 512)
            r = 512;

        if ((r == 512) && off > 0)
            r -= off;

        if (fno->off + r > fno->size)
            r = fno->size - fno->off;

        disk_read(fsd, (buf + r_len), (priv->sect + sect), off, r);

        r_len += r;
        fno->off += r;
        off += r;

        if ((r_len < len) && (off == fs->bps) && (fno->off < fno->size)) {
            sect++;
            if ((sect + 1) > fs->spc) {
                clust++;
                priv->cclust = priv->fat[clust];
                priv->sect = CLUST2SECT(fs, priv->cclust);
                sect = 0;
            }
        }
        off = 0;
    }

    return r_len;
}

int fatfs_write(struct fnode *fno, const void *buf, unsigned int len)
{
    if (!fno || !fno->priv)
        return -1;

    struct fatfs_priv *priv = (struct fatfs_priv *)fno->priv;
    struct fatfs_disk *fsd = priv->fsd;
    struct fatfs *fs = fsd->fs;

    int w_len = 0, sect = 0, off = 0, clust = 0, tmpclust = 0;

    /* calculate where to put sect and off! */
    off = fno->off;
    while (off >= fs->bps) {
        sect++;
        off -= fs->bps;
    }

    while (sect >= fs->spc) {
        clust++;
        sect -= fs->spc;
    }

    tmpclust = priv->sclust;
    while(clust > 0) {
        /* walk cluster chain until we have right cluster! */
        tmpclust = get_fat(fsd, tmpclust);
        clust--;
    }

    priv->cclust = tmpclust;
    priv->sect = CLUST2SECT(fs, priv->cclust);

    while (w_len < len) {
        disk_read(fsd, fs->win, (priv->sect + sect), 0, fs->bps);

        for (; w_len < len && off < fs->bps; w_len++) {
            //*(fs->win + off++) = *(uint8_t *)buf++;
            memcpy((fs->win + off++), (buf++), 1);
        }
        disk_write(fsd, fs->win, (priv->sect + sect), 0, fs->bps);
        fno->off += off;
        off = 0;
        if ((w_len < len) && (off == fs->bps)) {
            sect++;
            if ((sect + 1) > fs->spc) {
                uint32_t tempclus = priv->cclust;
                priv->cclust = init_fat(fsd);
                set_fat(fsd, tempclus, priv->cclust);
                priv->sect = CLUST2SECT(fs, priv->cclust);
                sect = 0;
            }
        }
    }

    if (fno->off > fno->size) {
        fno->size = fno->off;
        disk_read(fsd, fs->win, priv->dirsect, 0, fs->bps);
        st_dword((fs->win + priv->off + DIR_FSIZE), (uint32_t)fno->size);
        disk_write(fsd, fs->win, priv->dirsect, 0, fs->bps);
    }

    return w_len;
}

int fatfs_seek(struct fnode *fno, int off, int whence)
{
    struct fatfs_fnode *mfno;
    int new_off;

//    mfno = FNO_MOD_PRIV(fno, &mod_fatfs);
//    if (!mfno)
//        return -1;

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

    if (new_off > fno->size)
        return -1;

    fno->off = new_off;

    return fno->off;
}

int fatfs_close(struct fnode *fno)
{
    fno->off = 0;

    return 0;
}

int fatfs_init(void)
{
    mod_fatfs.family = FAMILY_FILE;
    strcpy(mod_fatfs.name,"fatfs");

    mod_fatfs.mount = fatfs_mount;
    mod_fatfs.ops.read = fatfs_read;
    mod_fatfs.ops.close = fatfs_close;
    mod_fatfs.ops.creat = fatfs_create;
    mod_fatfs.ops.write = fatfs_write;
    mod_fatfs.ops.seek = fatfs_seek;
    //mod_fatfs.ops.poll = fatfs_poll;

    /*
    mod_fatfs.ops.unlink = fatfs_unlink;
    mod_fatfs.ops.exe = fatfs_exe;
    */

    register_module(&mod_fatfs);
    return 0;
}


