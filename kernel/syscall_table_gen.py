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
    ["setclock", 1, "sys_setclock_hdlr"],
    ["sleep", 1, "sys_sleep_hdlr"],
    ["suspend", 1, "sys_suspend_hdlr"],
    ["thread_create", 3, "sys_thread_create_hdlr"],
    ["test", 5, "sys_test_hdlr"],
    ["getpid", 0, "sys_getpid_hdlr"],
    ["getppid", 0, "sys_getppid_hdlr"],
    ["open", 2, "sys_open_hdlr"],
    ["close", 1, "sys_close_hdlr"],
    ["read", 3, "sys_read_hdlr"],
    ["write", 3, "sys_write_hdlr"],
    ["seek", 3, "sys_seek_hdlr"],
    ["mkdir", 1, "sys_mkdir_hdlr"],
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
    ["chdir", 1, "sys_chdir_hdlr"],
    ["getcwd", 2, "sys_getcwd_hdlr"],
    ["sem_init", 1, "sys_sem_init_hdlr"],
    ["sem_post", 1, "sys_sem_post_hdlr"],
    ["sem_wait", 1, "sys_sem_wait_hdlr"],
    ["sem_destroy", 1, "sys_sem_destroy_hdlr"],
    ["mutex_init", 0, "sys_mutex_init_hdlr"],
    ["mutex_unlock", 1, "sys_mutex_unlock_hdlr"],
    ["mutex_lock", 1, "sys_mutex_lock_hdlr"],
    ["mutex_destroy", 1, "sys_mutex_destroy_hdlr"],
    ["socket", 3, "sys_socket_hdlr"],
    ["kill", 2, "sys_kill_hdlr"],
    ["exit", 1, "sys_exit_hdlr"]
]

   #
  ###
 #####
#######
#################################################################
hdr = open("../include/syscall_table.h", "w")
code = open("syscall_table.c", "w")
vector_h = open("syscall_vector.h", "w")
vector_c = open("syscall_vector.c", "w")

hdr.write("/* The file syscall_table.h is auto generated. DO NOT EDIT, CHANGES WILL BE LOST. */\n/* If you want to add syscalls, use syscall_table_gen.py  */\n\n#include \"frosted.h\"\n\n")
code.write("/* The file syscall_table.c is auto generated. DO NOT EDIT, CHANGES WILL BE LOST. */\n/* If you want to add syscalls, use syscall_table_gen.py  */\n\n#include \"frosted.h\"\n#include \"syscall_table.h\"\n")

for n in range(len(syscalls)):
    name = syscalls[n][0]
    hdr.write("#define SYS_%s \t\t\t(%d)\n" % (name.upper(), n))
hdr.write("#define _SYSCALLS_NR %d\n" % len(syscalls))

for n in range(len(syscalls)):
    name = syscalls[n][0]
    tp = syscalls[n][1]
    code.write( "/* Syscall: %s(%d arguments) */\n" % (name, tp))
    if (tp == 0):
        code.write( "int sys_%s(void){\n" % name)
        code.write( "    syscall(SYS_%s, 0, 0, 0, 0, 0); \n" % name.upper())
        code.write( "}\n")
        code.write("\n")
    if (tp == 1):
        code.write( "int sys_%s(uint32_t arg1){\n" % name)
        code.write( "    syscall(SYS_%s, arg1, 0, 0, 0, 0); \n" % name.upper())
        code.write( "}\n")
        code.write("\n")
    if (tp == 2):
        code.write( "int sys_%s(uint32_t arg1, uint32_t arg2){\n" % name)
        code.write( "    syscall(SYS_%s, arg1, arg2, 0, 0, 0); \n" % name.upper())
        code.write( "}\n")
        code.write("\n")
    if (tp == 3):
        code.write( "int sys_%s(uint32_t arg1, uint32_t arg2, uint32_t arg3){\n" % name)
        code.write( "    syscall(SYS_%s, arg1, arg2, arg3, 0,  0); \n" % name.upper())
        code.write( "}\n")
        code.write("\n")
    if (tp == 4):
        code.write( "int sys_%s(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4){\n" % name)
        code.write( "    syscall(SYS_%s, arg1, arg2, arg3, arg4, 0); \n" % name.upper())
        code.write( "}\n")
        code.write("\n")
    if (tp == 5):
        code.write( "int sys_%s(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){\n" % name)
        code.write( "    syscall(SYS_%s, arg1, arg2, arg3, arg4, arg5); \n" % name.upper())
        code.write( "}\n")
        code.write("\n")



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

vector_c.write("#include <stdint.h>\n")
vector_c.write("/* Syscall table Vector array */ \n")
for s in syscalls:
    vector_c.write("extern int sys_%s( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t );\n" % s[0])
vector_c.write("int __attribute__((section(\"syscall_vector\"))) (*__syscall__[%d])( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t ) = {\n" % len(syscalls))

for n in range(len(syscalls) - 1):
    name = syscalls[n][0]
    vector_c.write("\tsys_%s,\n" % (name))
name = syscalls[-1][0]
vector_c.write("\tsys_%s\n" % (name))
vector_c.write("};\n")

vector_c.close()
