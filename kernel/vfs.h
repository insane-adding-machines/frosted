#ifndef INC_VFS
#define INC_VFS


#define VFS_TYPE_BIN  (0) /* flat binary */
#define VFS_TYPE_BFLT (1) /* bFLT binary */

struct vfs_info {
    int type;
    int pic;
    void (*init)(void *);
    void * allocated;
    uint32_t text_size;
    uint32_t data_size;
};


#endif /* INC_VFS */
