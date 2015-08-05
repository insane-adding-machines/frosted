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
    ["open", 3, "sys_open_hdlr"],
    ["close",1, "sys_close_hdlr"],
    ["read", 3, "sys_read_hdlr"],
    ["write", 3, "sys_write_hdlr"],
    ["gettimeofday",1, "sys_gettimeofday_hdlr"],
    ["malloc",1, "sys_malloc_hdlr"],
    ["free",1, "sys_free_hdlr"]
]

   #
  ###
 #####
#######
#################################################################
hdr = open("kernel/syscall_table.h", "w")
code = open("kernel/syscall_table.c", "w")

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
    code.write( "extern %s(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);\n" % call)
code.write("\n")

code.write("void syscalls_init(void) {\n")
for n in range(len(syscalls)):
    call = syscalls[n][2]
    code.write( "\tsys_register_handler(%d, %s);\n" % (n, call) );
code.write("}\n")
