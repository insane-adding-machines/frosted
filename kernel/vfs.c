#include "frosted.h"
#include <string.h>


/* ROOT entity ("/")
 *.
 */
static struct fnode FNO_ROOT = {
};

static void basename_r(const char *path, char *res)
{
    char *p;
    strncpy(res, path, strlen(path) - 1);
    p = res + strlen(res) - 1;
    while (p > res) {
        if (*p == '/') {
            *p = '\0';
            return;
        }
        p--;
    }
}

static char *filename(char *path)
{
    int len = strlen(path);
    char *p = path + len - 1;
    while (p > path) {
        if (*p == '/')
            return (p + 1);
        p--;
    }
    return path;
}

static struct fnode *fno_create_file(char *path)
{
    char *base = kalloc(strlen(path) + 1);
    struct module *owner = NULL;
    struct fnode *parent;
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
    return fno_create(owner, filename(path), parent);
}

static struct fnode *fno_create_dir(char *path)
{
    struct fnode *fno = fno_create_file(path);
    if (fno) {
        fno->flags |= FL_DIR;
    }
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

    return 1;
}


static struct fnode *_fno_search(const char *path, struct fnode *dir)
{
    struct fnode *cur;
    int check = 0;
    if (dir == NULL) 
        return NULL;

    check = path_check(path, dir->fname);

    /* Does not match, try another item */
    if (check == 0)
        return _fno_search(path, dir->next);

    /* Item is found! */
    if (check == 2)
        return dir;

    /* path is correct, need to walk more */
    return _fno_search(path_walk(path), dir->children);
}

struct fnode *fno_search(const char *path)
{
    struct fnode *cur = &FNO_ROOT;
    return _fno_search(path, cur);
}

static struct fnode *_fno_create(struct module *owner, const char *name, struct fnode *parent)
{
    struct fnode *fno = kalloc(sizeof(struct fnode));
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
    return fno;
}

struct fnode *fno_mkdir(struct module *owner, const char *name, struct fnode *parent)
{
    struct fnode *fno = _fno_create(owner, name, parent);
    fno->flags |= (FL_DIR | FL_RDWR);
    if (parent && parent->owner && parent->owner->ops.creat)
        parent->owner->ops.creat(fno);
    return fno;
}

void fno_unlink(struct fnode *fno)
{
    struct fnode *dir = fno->parent;

    if (fno && fno->owner && fno->owner->ops.unlink)
        fno->owner->ops.unlink(fno);

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


int sys_open_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    char *path = (char *)arg1;
    struct fnode *f;
    uint32_t flags = arg2;

    if ((flags & O_CREAT) == 0) {
        f = fno_search(path);
    } else {
        if (flags & O_EXCL) {
            f = fno_search(path);
            if (f != NULL)
                return -1; /* XXX: EEXIST */
        }
        if (flags & O_TRUNC) {
            fno_unlink(fno_search(path));
        }
        f = fno_create_file(path);
    }
    if (f == NULL)
       return -1; /* XXX: ENOENT */
    if (f->flags & FL_INUSE)
        return -1; /* XXX: EBUSY */
    if (f->flags & FL_DIR)
        return -1; /* XXX: is a dir */
    if (flags & O_APPEND) {
        f->off = f->size;
    }
    return task_filedesc_add(f); 
}

int sys_close_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    struct fnode *f = task_filedesc_get(arg1);
    if (f != NULL) {
        if (f->owner && f->owner->ops.close)
            f->owner->ops.close(arg1);
        task_filedesc_del(arg1);
        return 0;
    }
    return -1; /* XXX: EINVAL */
}
    
int sys_seek_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    struct fnode *fno = task_filedesc_get(arg1);
    if (fno && fno->owner->ops.seek) {
        fno->owner->ops.seek(arg1, arg2, arg3);
    } else return -1;
}

int sys_mkdir_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    if (fno_create_dir((char *)arg1))
        return 0;
    return -1;
}

int sys_unlink_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    fno_unlink(fno_search((char *)arg1));
    return 0;
}

static struct dirent readdir_retval;

int sys_opendir_hdlr(uint32_t arg1)
{
    struct fnode *fno = fno_search((char *)arg1);
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

int sys_readdir_hdlr(uint32_t arg1)
{
    struct fnode *fno = (struct fnode *)arg1;
    struct fnode *next = (struct fnode *)fno->off;
    if (!next) {
        return (int)NULL;
    }
    fno->off = (int)next->next;
    readdir_retval.d_ino = 0; /* TODO: populate with inode? */
    strncpy(readdir_retval.d_name, next->fname, 256);
    return (int)(&readdir_retval);
}

int sys_closedir_hdlr(uint32_t arg1)
{
    struct fnode *fno = (struct fnode *)arg1;
    fno->off = 0;
    fno->flags &= ~(FL_INUSE);
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
    dev = fno_mkdir(NULL, "dev", NULL);

    /* Hook modules */
    devnull_init(dev);
    devuart_init(dev);
    memfs_init();
}

