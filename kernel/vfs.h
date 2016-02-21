#ifndef INC_VFS
#define INC_VFS


#define VFS_TYPE_BIN  (0) /* flat binary */
#define VFS_TYPE_BFLT (1) /* bFLT binary */

struct vfs_info {
    int type;
    void (*init)(void *);
    void * allocated;
};


#endif /* INC_VFS */
