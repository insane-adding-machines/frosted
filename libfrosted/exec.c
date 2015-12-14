/*
 * Frosted version of exec.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_exec(char *cmd, char **args);
extern int sys_execb(void (*init)(void **), void **args);

int exec(char *cmd, char **args)
{
    return sys_exec(cmd, args);
}

int execve(char *cmd, char *argv[], char *envp[])
{
    (void)envp;
    return sys_exec(cmd, (char **)argv);
}

int execvp(char *cmd, char *argv[])
{
    return sys_exec(cmd, (char **)argv);
}

int execv(char *cmd, char *argv[])
{
    return sys_exec(cmd, (char **)argv);
}

int execb(void (*init)(void**), void **args)
{
    return sys_execb(init, args);
}

