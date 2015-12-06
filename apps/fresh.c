/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the Free Software Foundation.
 *      
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted_api.h"
#include "syscalls.h"
#include <string.h>
#include <stdio.h>

static const char str_prompt[]       = "[frosted (%s)]:";

#define FRESH_DEV "/dev/ttyS0"

static void ls(void *arg)
{
    char *fname;
    char *fname_start;
    struct dirent *ep;
    DIR *d;
    struct stat st;
    char type;
    int i;
    char ch_size[8] = "";

    fname_start = malloc(MAX_FILE);
    ep = malloc(sizeof(struct dirent));
    if (!ep || !fname_start)
        while(1);;

    getcwd(fname_start, MAX_FILE);

    d = opendir(fname_start);
    while(readdir(d, ep) == 0) {
        fname = fname_start;
        fname[0] = '\0';
        strncat(fname, fname_start, MAX_FILE);
        strncat(fname, "/", MAX_FILE);
        strncat(fname, ep->d_name, MAX_FILE);

        while (fname[0] == '/')
            fname++;

        if (stat(fname, &st) == 0) {
            printf(fname);
            printf( "\t");
            if (S_ISDIR(st.st_mode)) {
                type = 'd';
            } else if (S_ISLNK(st.st_mode)) {
                type = 'l';
            } else {
                snprintf(ch_size, 8, "%lu", st.st_size);
                type = 'f';
            }

            printf( "%c", type);
            printf( "    ");
            printf( ch_size);
            printf( "\r\n");
        }
    }
    closedir(d);
    free(ep);
    free(fname_start);
}

static char lastcmd[100] = "";


void fresh(void *arg) {
    int stdin_fileno, stdout_fileno, stderr_fileno;
    char pwd[MAX_FILE] = "";

    do {
        stdin_fileno = open(FRESH_DEV, O_RDONLY);
    } while (stdin_fileno < 0);

    do {
        stdout_fileno = open(FRESH_DEV, O_WRONLY);
    } while (stdout_fileno < 0);
    
    do {
        stderr_fileno = open(FRESH_DEV, O_WRONLY);
    } while (stderr_fileno < 0);

    printf("Welcome to Frosted!\r\n");
    mkdir("/home");
    mkdir("/home/test");
    chdir("/home/test");

    while (2>1)
    {
        char input[100];
        int len = 0;
        struct pollfd pfd;
        int out = stdout_fileno;
        int i;
        char term[20] = "(none)";
        pfd.fd = stdin_fileno;
        pfd.events = POLLIN;
        len = 0;
        
        ttyname_r(0, term, 20);

        printf(str_prompt, term);
        getcwd(pwd, MAX_FILE);
        printf( pwd);
        printf( "$ ");

        /* Blocking calls: WIP
        if (poll(&pfd, 1, 1000) <= 0)
            continue;
        */
        while(len < 100)
        {
            const char del = 0x08;
            int ret = read(stdin_fileno, input + len, 3);
            
            /* arrows */
            if ((ret == 3) && (input[len] == 0x1b)) {
                char dir = input[len + 1];
                if (strlen(lastcmd) == 0) {
                    continue;
                }

                while (len > 0) {
                    write(stdout_fileno, &del, 1);
                    printf( " ");
                    write(stdout_fileno, &del, 1);
                    len--;
                }
                printf( lastcmd);
                len = strlen(lastcmd);
                strcpy(input, lastcmd);
                continue;
            }

            if (ret > 3)
                continue;
            if ((ret == 1) && (input[len] >= 0x20 && input[len] <= 0x7e)) {
                /* Echo to terminal */
                write(stdout_fileno, &input[len], 1);
            }
            len += ret;

            if ((input[len-1] == 0xD))
                break; /* CR (\r\n) */
            if ((input[len-1] == 0x4)) {
                printf( "\r\n");
                len = 0;
                break;
            }

            /* tab */
            if ((input[len-1] == 0x09)) {
                len--;
                printf("\r\n");
                printf("Supported commands: help ls mkdir touch cat echo rm.\r\n");
                printf("\r\n");
                printf( str_prompt);
                getcwd(pwd, MAX_FILE);
                printf( pwd);
                printf( "$ ");
                printf( input);
                continue;
            }


            /* backspace */
            if ((input[len-1] == 127)) {
                if (len > 1) {
                    printf( "%c", &del);
                    printf( " ");
                    printf( "%c", &del);
                    len -= 2;
                }else {
                    len -=1;
                }
            }
        }
        printf("\r\n");
        if (len == 0)
            break;
        
        if (strlen(input) == 0)
            continue;
        /* First, check redirections */
        input[len - 1] = '\0';
        strcpy(lastcmd, input);
        for (i = 0; i < len; i++) {
            if (input[i] == '>') {
                int flags = O_WRONLY | O_CREAT;
                input[i] = '\0';
                i++;
                if (input[i] == '>')
                    flags |= O_APPEND;
                else
                    flags |= O_TRUNC;
                while (input[i] == '>' || input[i] == ' ')
                    i++;
                out = open(&input[i], flags);
                if (out < 0) {
                    printf("File not found.\r\n");
                }
            }
        }


        /* Find executable command */
        char *cmd = input;
        char *args = cmd;
        struct stat st;
        for (i = 0; i < len; i++) {
            if (args[i] == ' ') {
                args[i] = 0;
                args += (i + 1);
                break;
            }
        }
        if (args == cmd)
            args = NULL;

        if ((stat(cmd, &st) == 0) && ((st.st_mode & P_EXEC) == P_EXEC)) {
            int pid = exec(cmd, args);
            thread_join(pid, -1);
            continue;
        } else {
            /* Restore space */
            if (args != NULL)
                *(args - 1) = ' ';
        } 
        
        if (!strncmp(input, "ls", 2))
        {
            /*
            int ls_pid = thread_create(ls, NULL, 0);
            if (ls_pid > 0)
                thread_join(ls_pid);
            */
            ls(NULL);
        
        } else if (!strncmp(input, "ln", 2)) {
            char *file = input + 3;
            char *symlink = file;
            while (*symlink != ' ') {
                if (*symlink == 0) {
                    printf("File not found.\r\n");
                    continue;
                }
                symlink++;
            }
            while (*symlink == ' ')  {
                *symlink = '\0';
                symlink++;
            }
            if (link(file, symlink) < 0)
                printf("File not found.\r\n");
        } else if (!strncmp(input, "help", 4)) {
            printf("Supported commands: help ls mkdir touch cat echo rm.\r\n");
        } else if (!strncmp(input, "cd", 2)) {
            if (strlen(input) > 2) {
                char *arg = input + 3;
                if (chdir(arg) < 0) {
                    printf("Directory not found.\r\n");
                }
            }
        } else if (!strncmp(input, "rm", 2)) {
            if (strlen(input) > 2) {
                char *arg = input + 3;
                if (unlink(arg) < 0) {
                    printf("File not found.\r\n");
                }
            }
        } else if (!strncmp(input, "mkdir", 5)) {
            if (strlen(input) > 5) {
                char *arg = input + 6;
                if (mkdir(arg) < 0) {
                    printf("Directory not found.\r\n");
                }
            }
        } else if (!strncmp(input, "touch", 5)) {
            if (strlen(input) > 5) {
                int fd; 
                char *arg = input + 6;
                fd = open(arg, O_CREAT|O_TRUNC|O_EXCL);
                if (fd < 0) {
                    printf("File not found.\r\n");
                } else close(fd);
            }
        } else if (!strncmp(input, "echo", 4)) {
            if (strlen(input) > 4) {
                int fd; 
                char *arg = input + 5;
                write(out, arg, strlen(arg));
                write(out, "\r\n", 2);
            }
        } else if (!strncmp(input, "cat", 3)) {
            if (strlen(input) > 3) {
                char *arg = input + 4;
                int fd; 
                fd = open(arg, O_RDONLY);
                if (fd < 0) {
                    printf("File not found.\r\n");
                } else {
                    int r;
                    char buf[10];
                    do {
                        r = read(fd, buf, 10);
                        if (r > 0) {
                            write(out, buf, r);
                        }
                    } while (r > 0);
                    close(fd);
                }
            }
        } else if (strlen(input) > 0){
            printf("Unknown command. Try 'help'.\r\n");
        }
        if (stdout_fileno != out)
            close(out);
        out = stdout_fileno;

    }
    close(stdout_fileno);
}
