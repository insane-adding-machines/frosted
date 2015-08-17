#include "frosted.h"
#include <string.h>

static struct fnode *sysfs;
static struct module mod_sysfs;

struct sysfs_fnode {
    struct fnode *fnode;
    int (*do_read)(struct sysfs_fnode *sfs, void *buf, int len);
    int (*do_write)(struct sysfs_fnode *sfs, const void *buf, int len);
};

static int sysfs_check_fd(int fd, struct fnode **fno)
{
    *fno = task_filedesc_get(fd);
    
    if (!fno)
        return -1;

    if (fd < 0)
        return -1;
    if ((*fno)->owner != &mod_sysfs)
        return -1;
    if ((*fno)->priv == NULL)
        return -1;

    return 0;
}

static int sysfs_read(int fd, void *buf, unsigned int len)
{
    struct fnode *fno;
    struct sysfs_fnode *mfno;
    if (len <= 0)
        return len;

    if (sysfs_check_fd(fd, &fno))
        return -1;

    mfno = fno->priv;
    if (mfno->do_read) {
        return mfno->do_read(mfno, buf, len);
    }
    return -1;

}

static int sysfs_write(int fd, const void *buf, unsigned int len)
{
    struct fnode *fno;
    struct sysfs_fnode *mfno;
    if (len <= 0)
        return len;

    if (sysfs_check_fd(fd, &fno))
        return -1;

    mfno = fno->priv;
    if (mfno->do_write) {
        return mfno->do_write(mfno, buf, len);
    }
    return -1;

}

static int sysfs_poll(int fd, uint16_t events, uint16_t *revents)
{
    *revents = events;
    return 1;
}

static int sysfs_close(int fd)
{
    struct fnode *fno;
    struct sysfs_fnode *mfno;
    if (sysfs_check_fd(fd, &fno))
        return -1;
    mfno = fno->priv;
    fno->off = 0;
    return 0;
}

int sysfs_time_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    char *res = (char *)buf;
    snprintf(res, len, "%lu\n", jiffies);
    return strlen(res);
}

int sysfs_time_write(struct sysfs_fnode *sfs, const void *buf, int len)
{
    return -1;
}


int sysfs_register(char *name, 
        int (*do_read)(struct sysfs_fnode *sfs, void *buf, int len), 
        int (*do_write)(struct sysfs_fnode *sfs, const void *buf, int len) )
{
    struct fnode *fno = fno_create(&mod_sysfs, name, sysfs);
    struct sysfs_fnode *mfs = kalloc(sizeof(struct sysfs_fnode));
    if (mfs) {

        mfs->fnode = fno;
        fno->priv = mfs;
        mfs->do_read = do_read;
        mfs->do_write = do_write;
        return 0;
    }
    return -1;
}

void sysfs_init(void)
{
    mod_sysfs.family = FAMILY_FILE;
    mod_sysfs.ops.read = sysfs_read; 
    mod_sysfs.ops.poll = sysfs_poll;
    mod_sysfs.ops.write = sysfs_write;
    mod_sysfs.ops.close = sysfs_close;

    sysfs = fno_mkdir(&mod_sysfs, "sys", NULL);
    register_module(&mod_sysfs);
    sysfs_register("time", sysfs_time_read, sysfs_time_write);
}




