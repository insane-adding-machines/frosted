/*
 * Frosted version of mount.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(char * source, char *target, char *module, uint32_t flags, void *args);

int mount(char * source, char *target, char *module, uint32_t flags, void *args)
{
    (void)flags; /* flags unimplemented for now */
    return __syscall__[SYS_MOUNT](source, target, module, flags, args);
}
