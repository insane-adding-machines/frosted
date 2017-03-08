#!/usr/bin/python2
#
#
#
# Add your syscalls here
#
# Syntax: [ "name", number_of_args, "name_of_kernelspace_handler" ]
#######
 #####
  ###
   #

syscalls = [
    ["sleep", 2, "sys_sleep_hdlr"],
    ["suspend", 1, "sys_suspend_hdlr"],
    ["getpid", 0, "sys_getpid_hdlr"],
    ["getppid", 0, "sys_getppid_hdlr"],
    ["open", 3, "sys_open_hdlr"],
    ["close", 1, "sys_close_hdlr"],
    ["read", 3, "sys_read_hdlr"],
    ["write", 3, "sys_write_hdlr"],
    ["seek", 3, "sys_seek_hdlr"],
    ["mkdir", 2, "sys_mkdir_hdlr"],
    ["unlink", 1, "sys_unlink_hdlr"],
    ["gettimeofday",1, "sys_gettimeofday_hdlr"],
    ["malloc", 1, "sys_malloc_hdlr"],
    ["free", 1, "sys_free_hdlr"],
    ["calloc", 2, "sys_calloc_hdlr"],
    ["realloc", 2, "sys_realloc_hdlr"],
    ["opendir", 1, "sys_opendir_hdlr"],
    ["readdir", 2, "sys_readdir_hdlr"],
    ["closedir",1, "sys_closedir_hdlr"],
    ["stat", 2, "sys_stat_hdlr"],
    ["poll", 3, "sys_poll_hdlr"],
    ["ioctl", 3, "sys_ioctl_hdlr"],
    ["link", 2, "sys_link_hdlr"],
    ["chdir", 1, "sys_chdir_hdlr"],
    ["getcwd", 2, "sys_getcwd_hdlr"],
    ["sem_init", 2, "sys_sem_init_hdlr"],
    ["sem_post", 1, "sys_sem_post_hdlr"],
    ["sem_wait", 2, "sys_sem_wait_hdlr"],
    ["sem_trywait", 1, "sys_sem_trywait_hdlr"],
    ["sem_destroy", 1, "sys_sem_destroy_hdlr"],
    ["mutex_init", 0, "sys_mutex_init_hdlr"],
    ["mutex_unlock", 1, "sys_mutex_unlock_hdlr"],
    ["mutex_lock", 1, "sys_mutex_lock_hdlr"],
    ["mutex_destroy", 1, "sys_mutex_destroy_hdlr"],
    ["socket", 3, "sys_socket_hdlr"],
    ["bind", 2, "sys_bind_hdlr"],
    ["accept", 2, "sys_accept_hdlr"],
    ["connect", 2, "sys_connect_hdlr"],
    ["listen", 2, "sys_listen_hdlr"],
    ["sendto", 5, "sys_sendto_hdlr"],
    ["recvfrom", 5, "sys_recvfrom_hdlr"],
    ["setsockopt", 5, "sys_setsockopt_hdlr"],
    ["getsockopt", 5, "sys_getsockopt_hdlr"],
    ["shutdown", 2, "sys_shutdown_hdlr"],
    ["dup", 1, "sys_dup_hdlr"],
    ["dup2", 2, "sys_dup2_hdlr"],
    ["mount", 5, "sys_mount_hdlr"],
    ["umount", 2, "sys_umount_hdlr"],
    ["kill", 2, "sys_kill_hdlr"],
    ["isatty", 1, "sys_isatty_hdlr"],
    ["exec", 2, "sys_exec_hdlr"],
    ["ttyname_r", 3, "sys_ttyname_hdlr"],
    ["exit", 1, "sys_exit_hdlr"],
    ["tcsetattr", 3, "sys_tcsetattr_hdlr"],
    ["tcgetattr", 2, "sys_tcgetattr_hdlr"],
    ["tcsendbreak", 2, "sys_tcsendbreak_hdlr"],
    ["pipe2", 2, "sys_pipe2_hdlr"],
    ["sigaction", 3, "sys_sigaction_hdlr"],
    ["sigprocmask", 3, "sys_sigprocmask_hdlr"],
    ["sigsuspend", 1, "sys_sigsuspend_hdlr"],
    ["vfork", 0, "sys_vfork_hdlr"],
    ["waitpid", 3, "sys_waitpid_hdlr"],
    ["lstat", 2, "sys_lstat_hdlr"],
    ["uname", 1, "sys_uname_hdlr"],
    ["getaddrinfo", 4, "sys_getaddrinfo_hdlr"],
    ["freeaddrinfo", 1, "sys_freeaddrinfo_hdlr"],
    ["fstat", 2, "sys_fstat_hdlr"],
    ["getsockname", 2, "sys_getsockname_hdlr"],
    ["getpeername", 2, "sys_getpeername_hdlr"],
    ["readlink", 3, "sys_readlink_hdlr"],
    ["fcntl", 3, "sys_fcntl_hdlr"],
    ["setsid", 0, "sys_setsid_hdlr"],
    ["ptrace", 4, "sys_ptrace_hdlr"],
    ["reboot", 0, "sys_reboot_hdlr"],
    ["getpriority", 2, "sys_getpriority_hdlr"],
    ["setpriority", 3, "sys_setpriority_hdlr"],
    ["ftruncate", 2, "sys_ftruncate_hdlr"],
    ["truncate", 2, "sys_truncate_hdlr"],
    ["pthread_create", 4, "sys_pthread_create_hdlr"],
    ["pthread_exit", 1, "sys_pthread_exit_hdlr"],
    ["pthread_join", 2, "sys_pthread_join_hdlr"],
    ["pthread_detach", 1, "sys_pthread_detach_hdlr"],
    ["pthread_cancel", 1, "sys_pthread_cancel_hdlr"],
    ["pthread_self", 0, "sys_pthread_self_hdlr"],
    ["pthread_setcancelstate", 2, "sys_pthread_setcancelstate_hdlr"],
    ["sched_yield", 0, "sys_sched_yield_hdlr"],
    ["pthread_mutex_init", 2, "sys_pthread_mutex_init_hdlr"],
    ["pthread_mutex_destroy", 1, "sys_pthread_mutex_destroy_hdlr"],
    ["pthread_mutex_lock", 1, "sys_pthread_mutex_lock_hdlr"],
    ["pthread_mutex_trylock", 1, "sys_pthread_mutex_trylock_hdlr"],
    ["pthread_mutex_unlock", 1, "sys_pthread_mutex_unlock_hdlr"],
    ["pthread_kill", 2, "sys_pthread_kill_hdlr"],
    ["clock_settime", 1, "sys_clock_settime_hdlr"],
]

   #
  ###
 #####
#######
#################################################################
hdr = open("kernel/frosted-headers/include/sys/frosted.h", "w")
usercode = open("kernel/frosted-headers/sys/frosted_syscalls.c", "w")
code = open("kernel/syscall_table.c", "w")

hdr.write("/* The file frosted.h is auto generated. DO NOT EDIT, CHANGES WILL BE LOST. */\n/* If you want to add syscalls, use syscall_table_gen.py  */\n\n")
usercode.write("/* The file frosted_syscalls.c is auto generated. DO NOT EDIT, CHANGES WILL BE LOST. */\n/* If you want to add syscalls, use syscall_table_gen.py  */\n\n#include <stdint.h>\n#include \"sys/frosted.h\"\n")
code.write("/* The file syscall_table.c is auto generated. DO NOT EDIT, CHANGES WILL BE LOST. */\n/* If you want to add syscalls, use syscall_table_gen.py  */\n\n#include \"frosted.h\"\n#include \"sys/frosted.h\"\n")

for n in range(len(syscalls)):
    name = syscalls[n][0]
    hdr.write("#define SYS_%s \t\t\t(%d)\n" % (name.upper(), n))
hdr.write("#define _SYSCALLS_NR (%d) /* We have %d syscalls! */\n" % (len(syscalls), len(syscalls)))

for n in range(len(syscalls)):
    name = syscalls[n][0]
    tp = syscalls[n][1]
    usercode.write( "/* Syscall: %s(%d arguments) */\n" % (name, tp))
    if (tp == 0):
        usercode.write( "int sys_%s(void){\n" % name)
        usercode.write( "    return syscall(SYS_%s, 0, 0, 0, 0, 0); \n" % name.upper())
        usercode.write( "}\n")
        usercode.write("\n")
    if (tp == 1):
        usercode.write( "int sys_%s(uint32_t arg1){\n" % name)
        usercode.write( "    return syscall(SYS_%s, arg1, 0, 0, 0, 0); \n" % name.upper())
        usercode.write( "}\n")
        usercode.write("\n")
    if (tp == 2):
        usercode.write( "int sys_%s(uint32_t arg1, uint32_t arg2){\n" % name)
        usercode.write( "    return syscall(SYS_%s, arg1, arg2, 0, 0, 0); \n" % name.upper())
        usercode.write( "}\n")
        usercode.write("\n")
    if (tp == 3):
        usercode.write( "int sys_%s(uint32_t arg1, uint32_t arg2, uint32_t arg3){\n" % name)
        usercode.write( "    return syscall(SYS_%s, arg1, arg2, arg3, 0,  0); \n" % name.upper())
        usercode.write( "}\n")
        usercode.write("\n")
    if (tp == 4):
        usercode.write( "int sys_%s(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4){\n" % name)
        usercode.write( "    return syscall(SYS_%s, arg1, arg2, arg3, arg4, 0); \n" % name.upper())
        usercode.write( "}\n")
        usercode.write("\n")
    if (tp == 5):
        usercode.write( "int sys_%s(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){\n" % name)
        usercode.write( "    return syscall(SYS_%s, arg1, arg2, arg3, arg4, arg5); \n" % name.upper())
        usercode.write( "}\n")
        usercode.write("\n")

usercode.close()


code.write("/* External handlers (defined elsewhere) : */ \n")
for n in range(len(syscalls)):
    name = syscalls[n][0]
    call = syscalls[n][2]
    code.write( "extern int %s(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);\n" % call)
code.write("\n")

code.write("void syscalls_init(void) {\n")
for n in range(len(syscalls)):
    call = syscalls[n][2]
    code.write( "\tsys_register_handler(%d, %s);\n" % (n, call) );
code.write("}\n")
code.close()
