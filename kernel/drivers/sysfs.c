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
 *      Authors:
 *
 */
 
#include "frosted.h"
#include "string.h"
#include "scheduler.h"

#define MAX_SYSFS_BUFFER 512

static struct fnode *sysfs;
static struct module mod_sysfs;

static frosted_mutex_t *sysfs_mutex = NULL;

extern struct mountpoint *MTAB;
extern struct f_malloc_stats f_malloc_stats[3];


void sysfs_lock(void)
{
    if (sysfs_mutex)
        frosted_mutex_lock(sysfs_mutex);
}

void sysfs_unlock(void)
{
    if (sysfs_mutex)
        frosted_mutex_unlock(sysfs_mutex);
}

static int sysfs_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct sysfs_fnode *mfno;
    if (len <= 0)
        return len;

    mfno = FNO_MOD_PRIV(fno, &mod_sysfs);
    if (!mfno)
        return -1;

    if (mfno->do_read) {
        return mfno->do_read(mfno, buf, len);
    }
    return -1;

}

static int sysfs_write(struct fnode *fno, const void *buf, unsigned int len)
{
    struct sysfs_fnode *mfno;
    if (len <= 0)
        return len;

    mfno = FNO_MOD_PRIV(fno, &mod_sysfs);
    if (mfno)
        return -1;

    if (mfno->do_write) {
        return mfno->do_write(mfno, buf, len);
    }
    return -1;

}

static int sysfs_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    *revents = events;
    return 1;
}

static int sysfs_close(struct fnode *fno)
{
    struct sysfs_fnode *mfno;
    mfno = FNO_MOD_PRIV(fno, &mod_sysfs);
    if (!mfno)
        return -1;
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
    res[fno->off++] = '\r';
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
    char *name;
    int p_state;
    const char legend[]="pid\tstate\tstack\tname\r\n";
    if (fno->off == 0) {
        frosted_mutex_lock(sysfs_mutex);
        task_txt = kalloc(MAX_SYSFS_BUFFER);
        if (!task_txt)
            return -1;
        off = 0;

        strcpy(task_txt, legend);
        off += strlen(legend);

        for (i = 1; i < MAX_SYSFS_BUFFER; i++) {
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
                if (p_state == TASK_FORKED)
                    task_txt[off++] = 'F';
                if (p_state == TASK_ZOMBIE)
                    task_txt[off++] = 'Z';

                task_txt[off++] = '\t';
                stack_used = scheduler_stack_used(i);
                off += ul_to_str(stack_used, task_txt + off);

                task_txt[off++] = '\t';
                name = scheduler_task_name(i);
                if (name)
                {
                    strcpy(&task_txt[off], name);
                    off += strlen(name);
                }

                task_txt[off++] = '\r';
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

#ifdef CONFIG_TCPIP_MEMPOOL
#   define NPOOLS 4
#else
#   define NPOOLS 3
#endif

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
        const char mem_stat_banner[NPOOLS][50] = {"\r\nKernel memory statistics\r\n",
                                          "\r\n\nUser memory statistics\r\n",
                                          "\r\n\nTask space statistics\r\n",
#ifdef CONFIG_TCPIP_MEMPOOL
                                          "\r\n\nTCP/IP space statistics\r\n",
#endif

        };

        const char malloc_banner[] = "\tObjects in use: ";
        const char mem_banner[] = "\tMemory in use: ";
        const char frags_banner[] = "\tReserved: ";
        int i;
        frosted_mutex_lock(sysfs_mutex);
        mem_txt = kalloc(MAX_SYSFS_BUFFER);
        if (!mem_txt)
            return -1;
        off = 0;

        for (i = 0; i < NPOOLS; i++) {
            unsigned long allocated = f_malloc_stats[i].malloc_calls - f_malloc_stats[i].free_calls;
            strcpy(mem_txt + off, mem_stat_banner[i]);
            off += strlen(mem_stat_banner[i]);
            strcpy(mem_txt + off, malloc_banner);
            off += strlen(malloc_banner);
            off += ul_to_str(allocated, mem_txt + off);
            *(mem_txt + off) = '\r';
            off++;
            *(mem_txt + off) = '\n';
            off++;

            strcpy(mem_txt + off, mem_banner);
            off += strlen(mem_banner);
            off += ul_to_str(f_malloc_stats[i].mem_allocated, mem_txt + off);

            *(mem_txt + off) = ' ';
            off++;
            *(mem_txt + off) = 'B';
            off++;
            *(mem_txt + off) = '\r';
            off++;
            *(mem_txt + off) = '\n';
            off++;

            strcpy(mem_txt + off, frags_banner);
            off += strlen(frags_banner);
            off += ul_to_str(mem_stats_frag(i), mem_txt + off);
            *(mem_txt + off) = ' ';
            off++;
            *(mem_txt + off) = 'B';
            off++;
            *(mem_txt + off) = '\r';
            off++;
            *(mem_txt + off) = '\n';
            off++;
        }
        if (off > 0)
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
        const char mod_banner[] = "Loaded modules:\r\n";
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

int sysfs_mtab_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    char *res = (char *)buf;
    struct fnode *fno = sfs->fnode;
    static char *mem_txt;
    static int off;
    int i;
    int stack_used;
    int p_state;
    struct mountpoint *m = MTAB;
    int l = 0;
    if (fno->off == 0) {
        const char mtab_banner[] = "Mountpoint\tDriver\t\tInfo\r\n--------------------------------------\r\n";
        frosted_mutex_lock(sysfs_mutex);
        mem_txt = kalloc(MAX_SYSFS_BUFFER);
        if (!mem_txt)
            return -1;
        off = 0;
        strcpy(mem_txt + off, mtab_banner);
        off += strlen(mtab_banner);

        while (m) {
            l = fno_fullpath(m->target, mem_txt + off, MAX_SYSFS_BUFFER - off);
            if (l > 0)
                off += l;
            *(mem_txt + (off++)) = '\t';
            *(mem_txt + (off++)) = '\t';

            if (m->target->owner) {
                strcpy(mem_txt + off, m->target->owner->name);
                off += strlen(m->target->owner->name);
            }
            *(mem_txt + (off++)) = '\t';
            *(mem_txt + (off++)) = '\t';

            l = 0;
            if (m->target->owner->mount_info) {
                l = m->target->owner->mount_info(m->target, mem_txt + off, MAX_SYSFS_BUFFER - off);
            }
            if (l > 0) {
                off += l;
            } else {
                strcpy(mem_txt + off, "None");
                off += 4;
            }
            *(mem_txt + (off++)) = '\r';
            *(mem_txt + (off++)) = '\n';

            m = m->next;
        }
        *(mem_txt + (off++)) = '\r';
        *(mem_txt + (off++)) = '\n';
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


int sysfs_register(char *name, char *dir,
        int (*do_read)(struct sysfs_fnode *sfs, void *buf, int len),
        int (*do_write)(struct sysfs_fnode *sfs, const void *buf, int len) )
{
    struct fnode *fno = fno_create(&mod_sysfs, name, fno_search(dir));
    struct sysfs_fnode *mfs;
    if (!fno)
        return -1;

    mfs = kalloc(sizeof(struct sysfs_fnode));
    if (mfs) {
        mfs->fnode = fno;
        fno->priv = mfs;
        mfs->do_read = do_read;
        mfs->do_write = do_write;
        return 0;
    }
    return -1;
}

static int sysfs_mount(char *source, char *tgt, uint32_t flags, void *args)
{
    struct fnode *tgt_dir = NULL;
    /* Source must be NULL */
    if (source)
        return -1;

    /* Target must be a valid dir */
    if (!tgt)
        return -1;

    tgt_dir = fno_search(tgt);

    if (!tgt_dir || ((tgt_dir->flags & FL_DIR) == 0)) {
        /* Not a valid mountpoint. */
        return -1;
    }

    /* TODO: check empty dir
    if (tgt_dir->children) {
        return -1;
    }
    */
    tgt_dir->owner = &mod_sysfs;
    sysfs_register("time", "/sys", sysfs_time_read, sysfs_no_write);
    sysfs_register("tasks","/sys",  sysfs_tasks_read, sysfs_no_write);
    sysfs_register("mem", "/sys", sysfs_mem_read, sysfs_no_write);
    sysfs_register("modules", "/sys", sysfs_modules_read, sysfs_no_write);
    sysfs_register("mtab", "/sys", sysfs_mtab_read, sysfs_no_write);
    return 0;
}

void sysfs_init(void)
{
    mod_sysfs.family = FAMILY_FILE;
    strcpy(mod_sysfs.name, "sysfs");

    mod_sysfs.mount = sysfs_mount;

    mod_sysfs.ops.read = sysfs_read;
    mod_sysfs.ops.poll = sysfs_poll;
    mod_sysfs.ops.write = sysfs_write;
    mod_sysfs.ops.close = sysfs_close;

    sysfs = fno_search("/sys");
    register_module(&mod_sysfs);
    fno_mkdir(&mod_sysfs, "net", sysfs);
    sysfs_mutex = frosted_mutex_init();
}
