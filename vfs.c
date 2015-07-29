#include "frosted.h"

static struct fnode FNO_ROOT = {
    .owner = NULL,
    .fname = "/",
    .mask = 0,
    .parent = &FNO_ROOT,
    .children = NULL,
    .next = NULL 
};


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

struct fnode *fno_create(struct module *owner, const char *name, struct fnode *parent)
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
    fno->mask = 0;
    return fno;
}

void fno_unlink(struct fnode *fno)
{
    struct fnode *dir = fno->parent;
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
