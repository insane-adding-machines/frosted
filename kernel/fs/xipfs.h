#ifndef XIPFS_INCLUDED
#define XIPFS_INCLUDED
#define XIPFS_MAGIC 0xC519FF55

#include <stdint.h>

struct xipfs_fat {
    uint32_t fs_magic;
    uint32_t fs_size;
    uint32_t fs_files;
    uint32_t timestamp;
};

struct xipfs_fhdr {
    uint32_t magic;
    char  name[56];
    uint32_t len;
    uint8_t  payload[0];
};

void xipfs_init(void);
#endif
