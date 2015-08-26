#include "frosted.h"
#include <string.h>

static struct fnode *sysfs;
static struct module mod_sysfs;

static mutex_t *sysfs_mutex;

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

static int ul_to_str(unsigned long n, char *s)
{
    int maxlen = 10;
    int i;
    int q = 1;

    if (n == 0) {
        s[0] = '0';
        s[1] = '\0';
        return 1;
    }

    for (i = 0; i <= maxlen; i++) {
        if ((n / q) == 0)
            break;
        q*=10;
    }
    q /= 10;
    s[i] = '\0';
    maxlen = i;

    for (i = 0; i < maxlen; i++) {
        int c;
        c = (n / q);
        s[i] = '0' + c;
        n -= (c * q);
        q /= 10;
    }
    return maxlen;
     
}

int sysfs_time_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    char *res = (char *)buf;
    struct fnode *fno = sfs->fnode;
    if (fno->off > 0)
        return -1;

    fno->off += ul_to_str(jiffies, res);
    res[fno->off++] = '\n';
    res[fno->off] = '\0';
    return fno->off;
}

#define MAX_TASKLIST 512
int sysfs_tasks_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    char *res = (char *)buf;
    struct fnode *fno = sfs->fnode;
    static char *task_txt;
    static int off;
    int i;
    int stack_used;
    int p_state;
    const char legend[]="pid\tstate\tstack used\n";
    if (fno->off == 0) {
        mutex_lock(sysfs_mutex);
        task_txt = kalloc(MAX_TASKLIST);
        if (!task_txt)
            return -1;
        off = 0;

        strcpy(task_txt, legend);
        off += strlen(legend);

        for (i = 1; i < scheduler_ntasks(); i++) {
            p_state = scheduler_task_state(i);
            if ((p_state != TASK_IDLE) && (p_state != TASK_OVER)) {
                off += ul_to_str(i, task_txt + off);
                task_txt[off++] = '\t';
                if (p_state == TASK_RUNNABLE)
                    task_txt[off++] = 'r';
                if (p_state == TASK_RUNNING)
                    task_txt[off++] = 'R';
                if (p_state == TASK_WAITING)
                    task_txt[off++] = 'w';
                if (p_state == TASK_SLEEPING)
                    task_txt[off++] = 's';
                
                task_txt[off++] = '\t';
                stack_used = scheduler_stack_used(i);
                off += ul_to_str(stack_used, task_txt + off);
                task_txt[off++] = '\n';
            }
        }
        task_txt[off++] = '\0';
    }
    if (off == fno->off) {
        kfree(task_txt);
        mutex_unlock(sysfs_mutex);
        return -1;
    }
    if (len > (off - fno->off)) {
       len = off - fno->off;
    }
    memcpy(res, task_txt + fno->off, len);
    fno->off += len;
    return len;
}

int sysfs_no_write(struct sysfs_fnode *sfs, const void *buf, int len)
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
    sysfs_mutex = mutex_init();
    sysfs_register("time", sysfs_time_read, sysfs_no_write);
    sysfs_register("tasks", sysfs_tasks_read, sysfs_no_write);
}




