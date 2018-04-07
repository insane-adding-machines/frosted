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
#include "gpio.h"
#include "lowpower.h"

#define MAX_SYSFS_BUFFER 1024

static struct fnode *sysfs;
static struct module mod_sysfs;

static mutex_t *sysfs_mutex = NULL;

extern struct mountpoint *MTAB;
extern struct f_malloc_stats f_malloc_stats[5];


void sysfs_lock(void)
{
    if (sysfs_mutex)
        mutex_lock(sysfs_mutex);
}

void sysfs_unlock(void)
{
    if (sysfs_mutex)
        mutex_unlock(sysfs_mutex);
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
    if (!mfno)
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
    return 0;
}

int ul_to_str(unsigned long n, char *s)
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


int nice_to_str(int8_t n, char *s)
{
    int i = 0;
    if (n == 0) {
        s[0] = '0';
        s[1] = '\n';
        return 1;
    }

    if (n == NICE_RT) {
        s[0] = 'R';
        s[1] = 'T';
        s[2] = '\0';
        return 2;
    }

    if (n < 0) {
        s[i++] = '-';
        n = 0 - n;
    }
    if (n > 20) {
        s[0] = 'E';
        s[1] = 'R';
        s[2] = 'R';
        s[3] = '\0';
        return 3;
    }

    if (n >= 10) {
        s[i++] = (n / 10) + '0';
    }
    s[i++] = (n % 10) + '0';
    s[i++] = '\0';
    return (i - 1);
}

int sysfs_time_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    char *res = (char *)buf;
    struct fnode *fno = sfs->fnode;
    uint32_t off = task_fd_get_off(fno);
    if (off > 0)
        return -1;

    off += ul_to_str(jiffies, res);

    res[off++] = '\r';
    res[off++] = '\n';
    res[off] = '\0';
    task_fd_set_off(fno, off);
    return off;
}

static uint32_t strtou32(const char *ptr) {
    long long val = 0;

    while(*ptr) {
        int v = *ptr;
        if ('0' <= v && v <= '9')
            v -= '0';

        val = val * 10 + v;
        ptr++;
    }
    return val;
}

static int sysfs_suspend_write(struct sysfs_fnode *sfs, const void *buf, int len)
{
    uint32_t interval = strtou32(buf);
    if (len >= 1 && interval >= 1)
        lowpower_sleep(0, interval);
    return len;
}

static int sysfs_standby_write(struct sysfs_fnode *sfs, const void *buf, int len)
{
    uint32_t interval = strtou32(buf);
    if (len >= 1 && interval >= 1)
        lowpower_sleep(1, interval);
    return len;
}


static int gpio_basename(const uint32_t base, char *name)
{
    switch(base) {
#if defined(STM32F4) || defined(STM32F7)
        case GPIOA: 
            strcpy(name,"GPIOA");
            return 5;
        case GPIOB: 
            strcpy(name,"GPIOB");
            return 5;
        case GPIOC: 
            strcpy(name,"GPIOC");
            return 5;
        case GPIOD: 
            strcpy(name,"GPIOD");
            return 5;
        case GPIOE: 
            strcpy(name,"GPIOE");
            return 5;
        case GPIOF: 
            strcpy(name,"GPIOF");
            return 5;
        case GPIOG: 
            strcpy(name,"GPIOG");
            return 5;
        case GPIOH: 
            strcpy(name,"GPIOH");
            return 5;
        case GPIOI: 
            strcpy(name,"GPIOI");
            return 5;
        case GPIOJ: 
            strcpy(name,"GPIOJ");
            return 5;
        case GPIOK: 
            strcpy(name,"GPIOK");
            return 5;
#endif
        default:
            strcpy(name,"N/A");
            return 3;
    }
}

#if defined(STM32F4) || defined(STM32F7)
int sysfs_pins_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    char *res = (char *)buf;
    struct fnode *fno = sfs->fnode;
    static char *txt;
    static uint32_t off;
    int i;
    int stack_used;
    char *name;
    int p_state;
    int nice;
    const char legend[]="Base\tPin\tMode\tDrive\tSpeed\tTrigger\tOwner\r\n";
    struct dev_gpio *g =  Gpio_list;
    uint32_t pin_n = 0;
    uint32_t cur_off = task_fd_get_off(fno);
    if (cur_off == 0) {
        mutex_lock(sysfs_mutex);
        txt = kalloc((1 + gpio_list_len()) * 80);
        if (!txt)
            return -1;
        off = 0;
        strcpy(txt, legend);
        off += strlen(legend);
        while (g) {

            /* Base */
            off += gpio_basename(g->base, txt + off);
            txt[off++] = '\t';
            
            /* Pin */
            pin_n = 0;
            while ((1 << pin_n) != g->pin)
                pin_n ++;
            off += ul_to_str(pin_n, txt + off);
            txt[off++] = '\t';

            /* Mode */
            switch (g->mode) {
                case GPIO_MODE_OUTPUT:
                    txt[off++] = 'O';
                    txt[off++] = 'U';
                    txt[off++] = 'T';
                    break;
                case GPIO_MODE_INPUT:
                    txt[off++] = 'I';
                    txt[off++] = 'N';
                    break;
                case GPIO_MODE_ANALOG:
                    txt[off++] = 'A';
                    txt[off++] = 'N';
                    txt[off++] = 'A';
                    txt[off++] = 'L';
                    txt[off++] = 'O';
                    txt[off++] = 'G';
                    break;
                case GPIO_MODE_AF:
                    txt[off++] = 'A';
                    txt[off++] = 'L';
                    txt[off++] = 'T';
                    off += ul_to_str(g->af, txt + off);
                    break;
            }
            txt[off++] = '\t';

            /* Drive */
            if (g->optype == GPIO_OTYPE_OD) {
                txt[off++] = 'O';
                txt[off++] = 'D';
                txt[off++] = 'r';
                txt[off++] = 'a';
                txt[off++] = 'i';
                txt[off++] = 'n';
            } else {
                switch(g->pullupdown) {
                    case GPIO_PUPD_PULLUP:
                        txt[off++] = 'P';
                        txt[off++] = 'u';
                        txt[off++] = 'l';
                        txt[off++] = 'l';
                        txt[off++] = 'U';
                        txt[off++] = 'p';
                        break;
                    case GPIO_PUPD_PULLDOWN:
                        txt[off++] = 'P';
                        txt[off++] = 'u';
                        txt[off++] = 'l';
                        txt[off++] = 'l';
                        txt[off++] = 'D';
                        txt[off++] = 'w';
                        break;
                    default:
                        txt[off++] = 'N';
                        txt[off++] = 'o';
                        txt[off++] = 'n';
                        txt[off++] = 'e';
                        break;
                }
            } 
            txt[off++] = '\t';


            /* Speed */
            /* FIXME: if you want to see output speed. */
            txt[off++] = '-';
            txt[off++] = '\t';

            /* Trigger */
            switch (g->trigger) {

                case GPIO_TRIGGER_RAISE:
                    txt[off++] = 'R';
                    txt[off++] = 'a';
                    txt[off++] = 'i';
                    txt[off++] = 's';
                    txt[off++] = 'e';
                    break;

                case GPIO_TRIGGER_FALL:
                    txt[off++] = 'F';
                    txt[off++] = 'a';
                    txt[off++] = 'l';
                    txt[off++] = 'l';
                    break;

                case GPIO_TRIGGER_TOGGLE:
                    txt[off++] = 'T';
                    txt[off++] = 'o';
                    txt[off++] = 'g';
                    txt[off++] = 'g';
                    txt[off++] = 'l';
                    txt[off++] = 'e';
                    break;

                default:
                    txt[off++] = 'N';
                    txt[off++] = 'o';
                    txt[off++] = 'n';
                    txt[off++] = 'e';
                    break;
            }
            txt[off++] = '\t';

            /* Owner's name */
            strcpy(txt + off, g->owner->name);
            off += strlen(g->owner->name);
            txt[off++] = '\r';
            txt[off++] = '\n';
            g = g->next;
        }
        txt[off++] = '\0';
    }
    cur_off = task_fd_get_off(fno);
    if (off == cur_off) {
        kfree(txt);
        mutex_unlock(sysfs_mutex);
        return -1;
    }
    if (len > (off - cur_off)) {
       len = off - cur_off;
    }
    memcpy(res, txt + cur_off, len);
    task_fd_set_off(fno, cur_off + len);
    return len;
}
#endif

int sysfs_tasks_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    char *res = (char *)buf;
    struct fnode *fno = sfs->fnode;
    static char *task_txt;
    static uint32_t off;
    int i;
    int stack_used;
    char *name;
    int p_state;
    int nice;
    const char legend[]="pid\tstate\tstack\theap\tnice\tname\r\n";
    uint32_t cur_off = task_fd_get_off(fno);
    if (cur_off == 0) {
        mutex_lock(sysfs_mutex);
        task_txt = kalloc(MAX_SYSFS_BUFFER);
        if (!task_txt)
            return -1;
        off = 0;

        strcpy(task_txt, legend);
        off += strlen(legend);

        for (i = 1; i <= 0xFFFF; i++) {
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
                if (p_state == TASK_STOPPED)
                    task_txt[off++] = 'S';

                task_txt[off++] = '\t';
                stack_used = scheduler_stack_used(i);
                off += ul_to_str(stack_used, task_txt + off);
                
                task_txt[off++] = '\t';
                stack_used = f_proc_heap_count(i);
                off += ul_to_str(stack_used, task_txt + off);
                
                task_txt[off++] = '\t';
                nice = scheduler_get_nice(i);
                off += nice_to_str(nice, task_txt + off);

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

    cur_off = task_fd_get_off(fno);
    if (off == cur_off) {
        kfree(task_txt);
        mutex_unlock(sysfs_mutex);
        return -1;
    }
    if (len > (off - cur_off)) {
       len = off - cur_off;
    }
    memcpy(res, task_txt + cur_off, len);
    cur_off += len;
    task_fd_set_off(fno, cur_off);
    return len;
}

#define NPOOLS 5

int sysfs_mem_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    char *res = (char *)buf;
    struct fnode *fno = sfs->fnode;
    static char *mem_txt;
    static int off;
    int i;
    int stack_used;
    int p_state;
    uint32_t cur_off = task_fd_get_off(fno);
    if (cur_off == 0) {
        const char mem_stat_banner[NPOOLS][50] = {"\r\nKernel memory statistics\r\n",
                                          "\r\n\nUser memory statistics\r\n",
                                          "\r\n\nTask space statistics\r\n",
                                          "\r\n\nTCP/IP space statistics\r\n",
                                          "\r\n\nExtra mem space statistics\r\n",

        };

        const char malloc_banner[] = "\tObjects in use: ";
        const char mem_banner[] = "\tMemory in use: ";
        const char frags_banner[] = "\tReserved: ";
        int i;
        mutex_lock(sysfs_mutex);
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
    cur_off = task_fd_get_off(fno);
    if (off == cur_off) {
        kfree(mem_txt);
        mutex_unlock(sysfs_mutex);
        return -1;
    }
    if (len > (off - cur_off)) {
       len = off - cur_off;
    }
    memcpy(res, mem_txt + cur_off, len);
    cur_off += len;
    task_fd_set_off(fno, cur_off);
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
    uint32_t cur_off = task_fd_get_off(fno);
    if (cur_off == 0) {
        const char mod_banner[] = "Loaded modules:\r\n";
        mutex_lock(sysfs_mutex);
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
    cur_off = task_fd_get_off(fno);
    if (off == cur_off) {
        kfree(mem_txt);
        mutex_unlock(sysfs_mutex);
        return -1;
    }
    if (len > (off - cur_off)) {
       len = off - cur_off;
    }
    memcpy(res, mem_txt + cur_off, len);
    cur_off += len;
    task_fd_set_off(fno,cur_off);
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
    uint32_t cur_off = task_fd_get_off(fno);
    if (cur_off == 0) {
        const char mtab_banner[] = "Mountpoint\tDriver\t\tInfo\r\n--------------------------------------\r\n";
        mutex_lock(sysfs_mutex);
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
    cur_off = task_fd_get_off(fno);
    if (off == cur_off) {
        kfree(mem_txt);
        mutex_unlock(sysfs_mutex);
        return -1;
    }
    if (len > (off - cur_off)) {
       len = off - cur_off;
    }
    memcpy(res, mem_txt + cur_off, len);
    cur_off += len;
    task_fd_set_off(fno,cur_off);
    return len;
}

int sysfs_no_write(struct sysfs_fnode *sfs, const void *buf, int len)
{
    return -1;
}

int sysfs_no_read(struct sysfs_fnode *sfs, void *buf, int len)
{
    return 0;
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
#if defined(STM32F4) || defined(STM32F7)
    sysfs_register("pins", "/sys", sysfs_pins_read, sysfs_no_write);
#endif
#ifdef CONFIG_LOWPOWER
    sysfs_register("suspend","/sys/power", sysfs_no_read, sysfs_suspend_write);
    sysfs_register("standby","/sys/power", sysfs_no_read, sysfs_standby_write);
#endif
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
    fno_mkdir(&mod_sysfs, "net", sysfs);
    fno_mkdir(&mod_sysfs, "power", sysfs);
    register_module(&mod_sysfs);
    sysfs_mutex = mutex_init();
}
