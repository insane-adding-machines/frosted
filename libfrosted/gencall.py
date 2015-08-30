#!/usr/bin/python

import sys

print "Enter the function signature: ",
sign = sys.stdin.readline().rstrip('\n')
part = sign.split('(')
if len(part) < 2:
    print "Wrong signature!"
    sys.exit(1)

desc = part[0].split(' ')

Name = desc[-1]
while Name[0] == '*':
    Name = Name.strip('*')
Retval = part[0].rstrip(Name)
print part[0]



args=[]
arguments = sign.strip(part[0])
arguments = arguments.strip('(')

while (arguments[-1] == ')'):
    arguments = arguments.rstrip(')')
    print arguments
for arg in arguments.split(','):
    print "arg's %s" % arg
    argname = arg.split(' ')[-1]
    print "argname's %s" % argname
    if argname[0] == '*':
        while argname[0] == '*':
            argname = argname.strip('*')
    print argname

    argtype = arg.rstrip(argname)
    args.append([argtype, argname])
    

call_list = ""
for a in args:
    call_list += a[1]
    if a != args[-1]:
        call_list+=', '


code = open(Name+".c", "w")
code.write('/*')
code.write('\n')
code.write('* Frosted version of %s' % Name)
code.write('\n')
code.write('*/')
code.write('\n')
code.write('\n')
code.write('\n')
code.write('#include "frosted_api.h"')
code.write('\n')
code.write('#include "syscall_table.h"')
code.write('\n')
code.write('#include <errno.h>')
code.write('\n')
code.write('#undef errno')
code.write('\n')
code.write('extern int errno;')
code.write('\n')
code.write('extern int (**__syscall__)(%s);'  % arguments)
code.write('\n')
code.write('\n')
code.write('\n')
code.write('%s' % sign)
code.write('\n')
code.write('{')
code.write('\n')
code.write('    return __syscall__[SYS_%s](%s);' % (Name.upper(), call_list))
code.write('\n')
code.write('}')
code.write('\n')






