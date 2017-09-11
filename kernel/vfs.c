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
#include "vfs.h"
#include "string.h"
#include "scheduler.h"
#include "sys/stat.h"

#define O_MODE(o) ((o & O_ACCMODE))
#define O_BLOCKING(f) ((f->flags & O_NONBLOCK) == 0)

struct mountpoint *MTAB = NULL;

/* ROOT entity ("/")
 *.
 */
static struct fnode FNO_ROOT = {
};

static void basename_r(const char *path, char *res)
{
    char *p;
    strncpy(res, path, strlen(path) + 1);
    p = res + strlen(res) - 1;
    while (p >= res) {
        if (*p == '/') {
            *p = '\0';
            break;
        }
        p--;
    }
    if (strlen(res) == 0) {
        res[0] = '/';
        res[1] = '\0';
    }

}

static char *filename(char *path)
{
    int len = strlen(path);
    char *p = path + len - 1;
    while (p >= path) {
        if (*p == '/')
            return (p + 1);
        p--;
    }
    return path;
}

static struct fnode *_fno_search(const char *path, struct fnode *dir, int follow);

static int _fno_fullpath(struct fnode *f, char *dst, char **p, int len)
{
    int nlen;
    if (!f)
        return -EINVAL;
    if ((f->flags & FL_LINK) == FL_LINK) {
        f =  _fno_search(f->linkname, &FNO_ROOT, 1);
    }
    if (f == &FNO_ROOT) {
        *p = dst + 1;
        dst[0] = '/';
        dst[1] = '\0';
        return 0;
    }
    if (!*p) {
        if (!f->parent)
            return -EINVAL; // what to do, how is this possible?
        _fno_fullpath(f->parent, dst, p, len);
    }
    nlen = strlen(f->fname);
    if (nlen + (*p - dst) > (len -1))
        return -ENAMETOOLONG;
    memcpy(*p, f->fname, nlen);
    *p += nlen;
    *(*p) = '/';
    *p += 1;
    *(*p + 1) = '\0';
    return 0;
}

int fno_fullpath(struct fnode *f, char *dst, int len)
{
    char *p = NULL;
    int ret;
    ret =  _fno_fullpath(f, dst, &p, len);
    if (ret == 0)  {
        int nlen = strlen(dst);
        if (nlen > 1) {
            /* Remove trailing "/" */
            dst[--nlen] = '\0';
            while (dst[nlen - 1] == '/') {
                dst[nlen - 1] = '\0';
                nlen--;
            }
        }
        return nlen;
    }
    return -ENOENT;
}

static int path_abs(char *src, char *dst, int len)
{
    struct fnode *f = task_getcwd();
    if (src[0] == '/')
        strncpy(dst, src, len);
    else {
        if (fno_fullpath(f, dst, len) > 0) {
            while (dst[strlen(dst) - 1] == '/')
                dst[strlen(dst) - 1] = '\0';

            strncat(dst, "/", len);
            strncat(dst, src, len);
            return 0;
        }
    }
    return 0;
}

static struct fnode *fno_create_file(char *path)
{
    char *base = kalloc(strlen(path) + 1);
    struct module *owner = NULL;
    struct fnode *parent;
    struct fnode *f = NULL;
    if (!base)
        return NULL;
    basename_r(path, base);
    parent = fno_search(base);
    kfree(base);
    if (!parent)
        return NULL;
    if ((parent->flags & FL_DIR) == 0)
        return NULL;

    if (parent) {
        owner = parent->owner;
    }
    f = fno_create(owner, filename(path), parent);
    if (f)
        f->flags = 0;
    return f;
}

static struct fnode *fno_link(char *src, char *dst)
{
    struct fnode *file;
    struct fnode *link;
    int file_name_len;
    char p_src[MAX_FILE];
    char p_dst[MAX_FILE];

    path_abs(src, p_src, MAX_FILE);
    path_abs(dst, p_dst, MAX_FILE);

    file = fno_search(p_src);
    if (!file)
        return NULL;

    link = fno_create_file(p_dst);
    if (!link)
        return NULL;

    file_name_len = strlen(p_src);

    link->flags |= FL_LINK;
    link->linkname = kalloc(file_name_len + 1);
    if (!link->linkname) {
        fno_unlink(link);
        return NULL;
    }
    strncpy(link->linkname, p_src, file_name_len + 1);
    return link;
}

int vfs_symlink(char *file, char *link)
{
    if (fno_link(file, link) != NULL)
        return 0;
    else return -EINVAL;
}

static void mkdir_links(struct fnode *fno)
{
    char path[MAX_FILE], selfl[MAX_FILE], parentl[MAX_FILE], path_basename[MAX_FILE];
    fno_fullpath(fno, path, MAX_FILE -4);
    strcpy(selfl, path);
    strcpy(parentl, path);
    strcat(selfl, "/.");
    strcat(parentl, "/..");
    if (fno) {
        fno_link(path, selfl);
        basename_r(path, path_basename);
        fno_link(path_basename, parentl);
    }
}

static struct fnode *fno_create_dir(char *path, uint32_t flags)
{
    struct fnode *fno = fno_create_file(path);
    if (fno) {
        fno->flags |= (FL_DIR | flags);
    }
    mkdir_links(fno);
    return fno;
}


static const char *path_walk(const char *path)
{
    const char *p = path;

    if (*p == '/') {
        while(*p == '/')
            p++;
        return p;
    }

    while ((*p != '\0') && (*p != '/'))
        p++;

    if (*p == '/')
        p++;

    if (*p == '\0')
        return NULL;

    return p;
}


/* Returns:
 * 0 = if path does not match
 * 1 = if path is in the right dir, need to walk more
 * 2 = if path is found!
 */

static int path_check(const char *path, const char *dirname)
{

    int i = 0;
    for (i = 0; dirname[i]; i++) {
        if (path[i] != dirname[i])
            return 0;
    }

    if (path[i] == '\0')
        return 2;

    if (path[i] == '/')
        return 1;

    if (i > 0 && (path[i - 1] == '/' && dirname[i - 1] == '/'))
        return 1;

    return 0;
}


static struct fnode *_fno_search(const char *path, struct fnode *dir, int follow)
{
    struct fnode *cur;
    char link[MAX_FILE];
    int check = 0;
    if (dir == NULL)
        return NULL;

    check = path_check(path, dir->fname);

    /* Does not match, try another item */
    if (check == 0) {
        if (!dir->next)
            return NULL;
        return _fno_search(path, dir->next, follow);
    }

    /* Item is found! */
    if (check == 2) {
        /* If it's a symlink, restart check */
        if (follow && ((dir->flags & FL_LINK) == FL_LINK)) {
            return _fno_search(dir->linkname, &FNO_ROOT, 1);
        }
        return dir;
    }

    /* path is correct, need to walk more */
    if( (dir->flags & FL_LINK ) == FL_LINK ){
    /* passing through a symlink */
        strcpy( link, dir->linkname );
        strcat( link, "/" );
        strcat( link, path_walk(path));
        return _fno_search( link, &FNO_ROOT, follow );
    }
    return _fno_search(path_walk(path), dir->children, follow);
}

struct fnode *fno_search(const char *_path)
{
    int i, len;
    struct fnode *fno = NULL;
    char *path = NULL;
    if (!_path)
        return NULL;

    len = strlen(_path);
    if (!len)
        return NULL;

    path = kcalloc(len + 1, 1);
    if (!path)
        return NULL;

    memcpy(path, _path, len + 1);

    i = len - 1;
    while (i > 0) {
        if (path[i] == '/')
            path[i] = '\0';
        else
            break;
    }
    if (strlen(path) > 0) {
        fno = _fno_search(path, &FNO_ROOT, 1);
    }
    kfree(path);
    return fno;
}

struct fnode *fno_search_nofollow(const char *path)
{
    return _fno_search(path, &FNO_ROOT, 0);
}

static struct fnode *_fno_create(struct module *owner, const char *name, struct fnode *parent)
{
    struct fnode *fno = kcalloc(sizeof(struct fnode), 1);
    int nlen = strlen(name);
    if (!fno)
        return NULL;

    fno->fname = kalloc(nlen + 1);
    if (!fno->fname){
        kfree(fno);
        return NULL;
    }

    memcpy(fno->fname, name, nlen + 1);
    if (!parent) {
        parent = &FNO_ROOT;
    }

    fno->parent = parent;
    fno->next = fno->parent->children;
    fno->parent->children = fno;

    fno->children = NULL;
    fno->owner = owner;
    return fno;
}

struct fnode *fno_create(struct module *owner, const char *name, struct fnode *parent)
{
    struct fnode *fno = _fno_create(owner, name, parent);
    if (fno && parent && parent->owner && parent->owner->ops.creat)
        parent->owner->ops.creat(fno);
    fno->flags |= FL_RDWR;
    return fno;
}

struct fnode *fno_create_wronly(struct module *owner, const char *name, struct fnode *parent)
{
    struct fnode *fno = _fno_create(owner, name, parent);
    if (fno && parent && parent->owner && parent->owner->ops.creat)
        parent->owner->ops.creat(fno);
    fno->flags = FL_WRONLY;
    return fno;
}

struct fnode *fno_create_rdonly(struct module *owner, const char *name, struct fnode *parent)
{
    struct fnode *fno = _fno_create(owner, name, parent);
    if (fno && parent && parent->owner && parent->owner->ops.creat)
        parent->owner->ops.creat(fno);
    fno->flags = FL_RDONLY;
    return fno;
}

struct fnode *fno_mkdir(struct module *owner, const char *name, struct fnode *parent)
{
    struct fnode *fno = _fno_create(owner, name, parent);
    fno->flags |= (FL_DIR | FL_RDWR);
    if (parent && parent->owner && parent->owner->ops.creat)
        parent->owner->ops.creat(fno);
    mkdir_links(fno);
    return fno;
}

void fno_unlink(struct fnode *fno)
{
    struct fnode *dir;

    if (!fno)
        return;
    dir = fno->parent;

    if (fno && fno->owner && fno->owner->ops.unlink)
        fno->owner->ops.unlink(fno);

    if (!fno)
        return;

    if (dir) {
        struct fnode *child = dir->children;
        while (child) {
            if (child == fno) {
                dir->children = fno->next;
                break;
            }
            if (child->next == fno) {
                child->next = fno->next;
                break;
            }
            child = child->next;
        }
    }


    kfree(fno->fname);
    kfree(fno);
}

int sys_readlink_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    char *path = (char*)arg1;
    char *buf = (char*)arg2;
    size_t size = (size_t)arg3;
    int len;
    char abs_p[MAX_FILE];
    struct fnode *fno;
    if(!buf)
        return -EINVAL;
    if (task_ptr_valid(buf) || task_ptr_valid(path))
        return -EACCES;
    path_abs(path, abs_p, MAX_FILE);
    fno = fno_search_nofollow(abs_p);
    if(!fno)
        return -ENOENT;
    else if (fno->flags & FL_LINK)
        strncpy(buf, fno->linkname, size);
    else
        return -EINVAL;
    len = strlen(fno->linkname);
    return len < size ? len : size;
}

int sys_exec_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    char *path = (char *)arg1;
    char *arg = (char *)arg2;
    struct fnode *f;
    struct vfs_info *vfsi = NULL;
    if (task_ptr_valid(arg) || task_ptr_valid(path))
        return -EACCES;

    f = fno_search(path);
    if (f && f->owner && (f->flags & FL_EXEC) && f->owner->ops.exe) {
        vfsi = (struct vfs_info *)f->owner->ops.exe(f, arg);
        if (vfsi)
            scheduler_exec(vfsi, arg);
    }
    return -EINVAL;
}

int sys_open_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    char *rel_path = (char *)arg1;
    struct fnode *f;
    uint32_t flags = arg2;
    char path[MAX_FILE];
    int ret;
    if (task_ptr_valid(rel_path))
        return -EACCES;

    path_abs(rel_path, path, MAX_FILE);
    f = fno_search(path);
    if (f && f->owner && f->owner->ops.open) {
        if ((O_MODE(flags) != O_RDONLY) && ((f->flags & FL_WRONLY)== 0))
            return -EPERM;
        ret = f->owner->ops.open(path, flags);
        if (ret >= 0)
            task_fd_setmask(ret, flags);
        return ret;
    }

    if ((flags & O_CREAT) == 0) {
        f = fno_search(path);
    } else {
        if ((O_MODE(flags)) == O_RDONLY)
            return -EPERM;
        f = fno_search(path);
        if (flags & O_EXCL) {
            if (f != NULL)
                return -EEXIST;
        }
        if (f && (flags & O_TRUNC)) {
            if (f) {
                fno_unlink(f);
                f = NULL;
            }

        }
        if (!f)
            f = fno_create_file(path);

        /* TODO: Parse arg3 & 0x1c0 for permissions */
        if (f)
            f->flags |= FL_RDWR;
    }
    if (f == NULL)
       return -ENOENT;
    if (f->flags & FL_INUSE)
        return -EBUSY;
    if (f->flags & FL_DIR)
        return -EISDIR;
    if (flags & O_APPEND) {
        f->off = f->size;
    } else {
        f->off = 0;
    }
    ret = task_filedesc_add(f);
    task_fd_setmask(ret, flags);
    return ret;
}

int sys_close_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    struct fnode *f = task_filedesc_get(arg1);
    if (f != NULL) {
        task_filedesc_del(arg1);
        return 0;
    }
    return -EINVAL;
}

int sys_seek_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    struct fnode *fno = task_filedesc_get(arg1);
    if (!fno)
        return -EINVAL;
    if (fno->owner->ops.seek) {
        return fno->owner->ops.seek(fno, arg2, arg3);
    } else return -EOPNOTSUPP;
}

int sys_ioctl_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    struct fnode *fno = task_filedesc_get(arg1);
    if (!fno)
        return -EINVAL;
    if (fno->owner->ops.ioctl) {
        fno->owner->ops.ioctl(fno, (uint32_t)arg2, (void *)arg3);
    } else return -EOPNOTSUPP;
}

int sys_link_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    struct fnode *fno;
    if (task_ptr_valid((void*)arg1) || task_ptr_valid((void*)arg2))
        return -EACCES;
    fno = fno_link((char*)arg1, (char *)arg2);
    if (fno)
        return 0;
    else return -EINVAL;
}


int sys_mkdir_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    char *path = (char *)arg1;
    char abs_p[MAX_FILE];
    struct fnode *f;
    if (task_ptr_valid(path))
        return -EACCES;
    path_abs(path, abs_p, MAX_FILE);
    if (fno_create_dir(abs_p, arg2))
        return 0;
    return -ENOENT;
}

int sys_unlink_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    char *path = (char *)arg1;
    char abs_p[MAX_FILE];
    struct fnode *f;
    if (task_ptr_valid(path))
        return -EACCES;
    path_abs(path, abs_p, MAX_FILE);
    f = fno_search_nofollow(abs_p); /* Don't follow symlink */
    if (f) {
        fno_unlink(f);
        return 0;
    }
    return -ENOENT;
}

int sys_opendir_hdlr(uint32_t arg1)
{
    struct fnode *fno;
    if (task_ptr_valid((void*)arg1))
        return -EACCES;
    fno = fno_search((char *)arg1);
    if (fno && (fno->flags & FL_DIR)) {
        if (fno->flags & FL_INUSE)
            return (int)NULL; /* XXX EBUSY */
        /* Use .off to store current readdir ptr */
        fno->off = (int)fno->children;
        fno->flags |= FL_INUSE;
        return (int)fno;
    } else {
        return (int)NULL;
    }
}

int sys_readdir_hdlr(uint32_t arg1, uint32_t arg2)
{
    struct fnode *fno;
    struct fnode *next;
    struct dirent *ep;

    fno = (struct fnode *)arg1;
    ep = (struct dirent *)arg2;
    if (task_ptr_valid(ep))
       return  -EACCES;

    next = (struct fnode *)fno->off;
    if (!fno || !ep)
        return -ENOENT;
    if (!next) {
        return -1;
    }
    fno->off = (int)next->next;
    ep->d_ino = 0; /* TODO: populate with inode? */
    strncpy(ep->d_name, next->fname, 256);
    return 0;
}

int sys_closedir_hdlr(uint32_t arg1)
{
    struct fnode *fno = (struct fnode *)arg1;
    fno->off = 0;
    fno->flags &= ~(FL_INUSE);
    return 0;
}


static int stat_hdlr(char *path, struct stat *st)
{
    char abs_p[MAX_FILE];
    struct fnode *fno;
    path_abs(path, abs_p, MAX_FILE);
    fno = fno_search_nofollow(abs_p);
    if (!fno)
        return -ENOENT;
    if (fno->flags & FL_DIR) {
        st->st_mode = S_IFDIR;
        st->st_size = 0;
    } else if (fno->flags & FL_TTY) {
        st->st_mode = S_IFCHR;
        st->st_size = 0;
    } else if (fno->flags & FL_BLK) {
        st->st_mode = S_IFBLK;
        st->st_size = 0;
    } else if (fno->flags & FL_LINK) {
        return stat_hdlr(fno->linkname, st); /* Stat follows symlink */
    } else {
        st->st_mode = S_IFREG;
        st->st_size = fno->size;
    }

    if (fno->flags & FL_EXEC) {
        st->st_mode |= P_EXEC;
    }
    return 0;
}

int sys_stat_hdlr(char *arg1, struct stat *arg2)
{
    char *path = arg1;
    struct stat *st = arg2;
    if (task_ptr_valid(path) || task_ptr_valid(st))
       return  -EACCES;
    return stat_hdlr(path, st);
}

int sys_fstat_hdlr(uint32_t arg1, struct stat *arg2)
{
    struct stat *st = arg2;
    struct fnode *fno = task_filedesc_get(arg1);
    if (!fno)
        return -ENOENT;
    if (task_ptr_valid(st))
       return  -EACCES;
    if (fno->flags & FL_DIR) {
        st->st_mode = S_IFDIR;
        st->st_size = 0;
    } else if (fno->flags & FL_LINK) {
        return stat_hdlr(fno->linkname, st); /* Stat follows symlink */
    } else {
        st->st_mode = S_IFREG;
        st->st_size = fno->size;
    }

    if (fno->flags & FL_EXEC) {
        st->st_mode |= P_EXEC;
    }
    return 0;
}

int sys_lstat_hdlr(char *arg1, struct stat *arg2)
{
    char *path = arg1;
    struct stat *st = arg2;
    char abs_p[MAX_FILE];
    struct fnode *fno;
    if (task_ptr_valid(st) || task_ptr_valid(path))
       return  -EACCES;
    path_abs(path, abs_p, MAX_FILE);
    fno = fno_search_nofollow(abs_p);
    if (!fno)
        return -ENOENT;
    if (fno->flags & FL_DIR) {
        st->st_mode = S_IFDIR;
        st->st_size = 0;
    } else if (fno->flags & FL_LINK) {
        st->st_mode = S_IFLNK; /* lstat gives info about the link itself */
        st->st_size = 0;
    } else {
        st->st_mode = S_IFREG;
        st->st_size = fno->size;
    }

    if (fno->flags & FL_EXEC) {
        st->st_mode |= P_EXEC;
    }
    return 0;
}

int vfs_truncate(struct fnode *fno, unsigned size)
{
    if (!fno)
        return -ENOENT;
    if ((fno->flags & FL_WRONLY) == 0)
        return -EPERM;
    if (!fno->owner ||  !fno->owner->ops.truncate)
        return -EOPNOTSUPP;
    return fno->owner->ops.truncate(fno, size);
}

int sys_ftruncate_hdlr(uint32_t arg1, unsigned arg2)
{
    struct fnode *fno = task_filedesc_get(arg1);
    return vfs_truncate(fno, arg2);
}

int sys_truncate_hdlr(char *arg1, unsigned arg2)
{
    char abs_p[MAX_FILE];
    struct fnode *fno;
    if (task_ptr_valid((void*)arg1))
        return -EACCES;
    path_abs(arg1, abs_p, MAX_FILE);
    fno = fno_search(abs_p);
    return vfs_truncate(fno, arg2);
}

int sys_chdir_hdlr(char *arg1)
{
    char *path = arg1;
    char abs_p[MAX_FILE];
    struct fnode *f;
    if (task_ptr_valid(path))
       return  -EACCES;
    path_abs(path, abs_p, MAX_FILE);

    f = fno_search(abs_p);
    if (!f || (!(f->flags & FL_DIR)))
        return -ENOTDIR;
    task_chdir(f);
    return 0;
}

int sys_isatty_hdlr(uint32_t arg1)
{
    struct fnode *f = task_filedesc_get(arg1);
    if (f && f->flags & FL_TTY)
        return 1;
    return 0;
}

int sys_ttyname_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    struct fnode *f = task_filedesc_get(arg1);
    if (task_ptr_valid((void*)arg2))
       return  -EACCES;
    if (f && f->flags & FL_TTY) {
        strncpy((char *)arg2, f->fname, arg3);
        return 0;
    }
    return -EBADF;
}

int sys_getcwd_hdlr(uint32_t arg1, uint32_t arg2)
{
    char *path = (char *)arg1;
    int len = (int)arg2;
    if (task_ptr_valid(path))
       return  -EACCES;
    if (fno_fullpath(task_getcwd(), path, len) > 0)
        return arg1;
    return 0;
}

void __attribute__((weak)) devnull_init(struct fnode *dev)
{

}

void __attribute__((weak)) memfs_init(void)
{

}

void __attribute__((weak)) xipfs_init(void)
{

}

void __attribute__((weak)) sysfs_init(void)
{

}

void __attribute__((weak)) fatfs_init(void)
{

}

void __attribute__((weak)) devgpio_init(struct fnode *dev)
{

}


void __attribute__((weak)) devuart_init(struct fnode *dev)
{

}

void __attribute__((weak)) devspi_init(struct fnode *dev)
{

}

int vfs_mount(char *source, char *target, char *module, uint32_t flags, void *args)
{
    struct module *m;
    if (!module || !target)
        return -ENOMEDIUM;
    m = module_search(module);
    if (!m || !m->mount)
        return -EOPNOTSUPP;
    if (m->mount(source, target, flags, args) == 0) {
        struct mountpoint *mp = kalloc(sizeof(struct mountpoint));
        if (mp) {
            mp->target = fno_search(target);
            mp->next = MTAB;
            MTAB = mp;
        }
        return 0;
    }
    return -ENOENT;
}

int vfs_umount(char *target, uint32_t flags)
{
    struct fnode *f;
    int ret;
    struct mountpoint *mp = MTAB, *prev = NULL;
    if (!target)
        return -ENOENT;
    f = fno_search(target);
    if (!f || !f->owner || !f->owner->umount)
        return -ENOMEDIUM;
    ret = f->owner->umount(target, flags);
    if (ret < 0)
        return ret;

    while (mp) {
        if (mp->target == f) {
            if (!prev) {
                MTAB = mp->next;
                break;
            } else {
                prev->next = mp;

            }
            kfree(mp);
        }
        prev = mp;
        mp = mp->next;
    }
    return 0;
}

int sys_mount_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    char *source = (char *)arg1;
    char *target = (char *)arg2;
    char *module = (char *)arg3;
    uint32_t flags = arg4;
    void *args = (void*)arg5;
    if (task_ptr_valid((void*)arg1) || task_ptr_valid((void*)arg2)|| task_ptr_valid((void*)arg3)|| (arg5 && task_ptr_valid((void*)arg5)))
       return  -EACCES;
    return vfs_mount(source, target, module, flags, args);
}

int sys_umount_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    if (task_ptr_valid((void*)arg1))
        return -EACCES;
    char *target = (char *)arg1;
    uint32_t flags = arg2;
    return vfs_umount(target, flags);

}

int sys_fcntl_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    int fd = arg1;
    int cmd = arg2;
    uint32_t fl_set = arg3;
    struct fnode *f = task_filedesc_get(arg1);
    if (!f) {
        return -EINVAL;
    }
    if (cmd == F_SETFL) {
        f->flags |= fl_set;
    } else if (cmd == F_GETFL) {
        return f->flags;
    }
    return 0;
}

void vfs_init(void)
{
    struct fnode *dev = NULL;
    /* Initialize "/" */
    FNO_ROOT.owner = NULL;
    FNO_ROOT.fname = "/";
    FNO_ROOT.parent = &FNO_ROOT;
    FNO_ROOT.children = NULL;
    FNO_ROOT.next = NULL ;
    FNO_ROOT.flags = FL_DIR | FL_RDWR;

    /* Init "/dev" dir */
    fno_mkdir(NULL, "dev", NULL);

    /* Init "/sys" dir */
    fno_mkdir(NULL, "sys", NULL);

    /* Init "/tmp" dir */
    fno_mkdir(NULL, "tmp", NULL);

    /* Init "/bin" dir */
    fno_mkdir(NULL, "bin", NULL);

    /* Init "/mnt" dir */
    fno_mkdir(NULL, "mnt", NULL);
}

