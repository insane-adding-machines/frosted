/*
 * Frosted version of exec.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(char *cmd, char **args);

int exec(char *cmd, char **args)
{
    return __syscall__[SYS_EXEC](cmd, args);
}

int execve(char *cmd, char *argv[], char *envp[])
{
    (void)envp;
    return __syscall__[SYS_EXEC](cmd, (char **)argv);
}
