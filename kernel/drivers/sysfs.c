#include "frosted.h"
#include <string.h>

#define MAX_SYSFS_BUFFER 512

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
        frosted_mutex_lock(sysfs_mutex);
        task_txt = kalloc(MAX_SYSFS_BUFFER);
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
                    task_txt[off++] = 'W';
                if (p_state == TASK_ZOMBIE)
                    task_txt[off++] = 'Z';
                
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
        frosted_mutex_unlock(sysfs_mutex);
        return -1;
    }
    if (len > (off - fno->off)) {
       len = off - fno->off;
    }
    memcpy(res, task_txt + fno->off, len);
    fno->off += len;
    return len;
}

int sysfs_mem_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    char *res = (char *)buf;
    struct fnode *fno = sfs->fnode;
    static char *mem_txt;
    static int off;
    int i;
    int stack_used;
    int p_state;
    if (fno->off == 0) {
        const char k_stat_banner[] = "\n\nKernel statistics\n";
        const char t_stat_banner[] = "\n\nTasks statistics\n";
        const char malloc_banner[] = "\tMalloc calls: ";
        const char free_banner[] = "\tFree calls: ";
        const char mem_banner[] = "\tMemory in use: ";
        frosted_mutex_lock(sysfs_mutex);
        mem_txt = kalloc(MAX_SYSFS_BUFFER);
        if (!mem_txt)
            return -1;
        off = 0;

        strcpy(mem_txt + off, k_stat_banner);
        off += strlen(k_stat_banner);
        strcpy(mem_txt + off, malloc_banner);
        off += strlen(malloc_banner);
        off += ul_to_str(f_malloc_stats[0].malloc_calls, mem_txt + off);
        *(mem_txt + off) = '\n'; 
        off++;
        strcpy(mem_txt + off, free_banner);
        off += strlen(free_banner);
        off += ul_to_str(f_malloc_stats[0].free_calls, mem_txt + off);
        *(mem_txt + off) = '\n'; 
        off++;

        strcpy(mem_txt + off, mem_banner);
        off += strlen(mem_banner);
        off += ul_to_str(f_malloc_stats[0].mem_allocated, mem_txt + off);
        *(mem_txt + off) = '\n'; 
        off++;
        
        strcpy(mem_txt + off, t_stat_banner);
        off += strlen(k_stat_banner);
        strcpy(mem_txt + off, malloc_banner);
        off += strlen(malloc_banner);
        off += ul_to_str(f_malloc_stats[1].malloc_calls, mem_txt + off);
        *(mem_txt + off) = '\n'; 
        off++;
        strcpy(mem_txt + off, free_banner);
        off += strlen(free_banner);
        off += ul_to_str(f_malloc_stats[1].free_calls, mem_txt + off);
        *(mem_txt + off) = '\n'; 
        off++;
        strcpy(mem_txt + off, mem_banner);
        off += strlen(mem_banner);
        off += ul_to_str(f_malloc_stats[1].mem_allocated, mem_txt + off);
        *(mem_txt + off) = '\n'; 
        off++;
        mem_txt[off++] = '\0';
    }
    if (off == fno->off) {
        kfree(mem_txt);
        frosted_mutex_unlock(sysfs_mutex);
        return -1;
    }
    if (len > (off - fno->off)) {
       len = off - fno->off;
    }
    memcpy(res, mem_txt + fno->off, len);
    fno->off += len;
    return len;
}

int sysfs_modules_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    char *res = (char *)buf;
    struct fnode *fno = sfs->fnode;
    static char *mem_txt;
    static int off;
    int i;
    int stack_used;
    int p_state;
    struct module *m = MODS;
    if (fno->off == 0) {
        const char mod_banner[] = "Loaded modules:\n";
        frosted_mutex_lock(sysfs_mutex);
        mem_txt = kalloc(MAX_SYSFS_BUFFER);
        if (!mem_txt)
            return -1;
        off = 0;
        strcpy(mem_txt + off, mod_banner);
        off += strlen(mod_banner);

        while (m) {
            strcpy(mem_txt + off, m->name);
            off += strlen(m->name);
            *(mem_txt + (off++)) = '\r';
            *(mem_txt + (off++)) = '\n';
            m = m->next;
        }
    }
    if (off == fno->off) {
        kfree(mem_txt);
        frosted_mutex_unlock(sysfs_mutex);
        return -1;
    }
    if (len > (off - fno->off)) {
       len = off - fno->off;
    }
    memcpy(res, mem_txt + fno->off, len);
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
    strcpy(mod_sysfs.name, "sysfs");

    mod_sysfs.ops.read = sysfs_read; 
    mod_sysfs.ops.poll = sysfs_poll;
    mod_sysfs.ops.write = sysfs_write;
    mod_sysfs.ops.close = sysfs_close;

    sysfs = fno_mkdir(&mod_sysfs, "sys", NULL);
    register_module(&mod_sysfs);
    sysfs_mutex = frosted_mutex_init();
    sysfs_register("time", sysfs_time_read, sysfs_no_write);
    sysfs_register("tasks", sysfs_tasks_read, sysfs_no_write);
    sysfs_register("mem", sysfs_mem_read, sysfs_no_write);
    sysfs_register("modules", sysfs_modules_read, sysfs_no_write);
}




