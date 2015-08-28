#include "frosted_api.h"
#include "syscalls.h"
#include <string.h>
#include <stdio.h>
static const char str_welcome[]      = "Welcome to Frosted\r\n";
static const char str_unknowncmd[]   = "Unknown command. Try 'help'.\r\n";
static const char str_invaliddir[]   = "Directory not found.\r\n";
static const char str_invalidfile[]  = "File not found.\r\n";
static const char str_help[]         = "Supported commands: help ls mkdir touch cat echo rm.\r\n";
static const char str_prompt[]       = "[frosted]:";

static void ls(int ser, char *start)
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

    d = opendir(start);
    while(readdir(d, ep) == 0) {
        fname = fname_start;
        fname[0] = '\0';
        strncat(fname, start, MAX_FILE);
        strncat(fname, "/", MAX_FILE);
        strncat(fname, ep->d_name, MAX_FILE);

        while (fname[0] == '/' && fname[1] == '/')
            fname++;

        stat(fname, &st);
        write(ser, fname, strlen(fname));
        write(ser, "\t", 1);
        if (S_ISDIR(st.st_mode))
            type = 'd';
        else {
            snprintf(ch_size, 8, "%lu", st.st_size);
            type = 'f';
        }

        write(ser, &type, 1); 
        write(ser, "    ", 4);
        write(ser, ch_size, strlen(ch_size));
        write(ser, "\n", 1);
    }
    closedir(d);
    free(ep);
    free(fname_start);
}

static char lastcmd[100] = "";

void fresh(void *arg) {
    int ser;
    char pwd[MAX_FILE] = "";

    do {
        ser = open("/dev/ttyS0", O_RDWR);
    } while (ser < 0);

    write(ser, str_welcome, strlen(str_welcome));
    mkdir("/home");
    mkdir("/home/test");
    chdir("/home/test");

    while (2>1)
    {
        char input[100];
        int len = 0;
        struct pollfd pfd;
        int out = ser;
        int i;
        pfd.fd = ser;
        pfd.events = POLLIN;
        len = 0;
        write(ser, str_prompt, strlen(str_prompt));
        getcwd(pwd, MAX_FILE);
        write(ser, pwd, strlen(pwd));
        write(ser, "$ ", 2);

        /* Blocking calls: WIP
        if (poll(&pfd, 1, 1000) <= 0)
            continue;
        */
        while(len < 100)
        {
            const char del = 0x08;
            int ret = read(ser, input + len, 1);

            if (ret != 1)
                continue;
            len += ret;
            if ((input[len-1] == 0xD))
                break; /* CR (\n) */
            if ((input[len-1] == 0x4)) {
                write(ser, "\n", 1);
                len = 0;
                break;
            }

            /* arrows */
            if (input[len-1] == 0x1b) {
                char dir;
                while (read(ser, &dir, 1) > 0) ;;
                write(ser, &del, 1);
                write(ser, " ", 1);
                write(ser, &del, 1);

                while (len > 0) {
                    write(ser, &del, 1);
                    write(ser, " ", 1);
                    write(ser, &del, 1);
                    len--;
                }
                write(ser, lastcmd, strlen(lastcmd));
                len = strlen(lastcmd);
                strcpy(input, lastcmd);
            }

            /* backspace */
            if ((input[len-1] == 127)) {
                if (len > 1) {
                    write(ser, &del, 1);
                    write(ser, " ", 1);
                    write(ser, &del, 1);
                    len -= 2;
                }else {
                    len -=1;
                }
            }
        }
        write(ser, "\n", 1);
        if (len == 0)
            break;

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
                    write(ser, str_invalidfile, strlen(str_invalidfile));
                }
            }
        }
        
        if (strlen(input) == 0)
            continue;
        
        if (!strncmp(input, "ls", 2))
        {
            ls(out, pwd);
        } else if (!strncmp(input, "help", 4)) {
            write(out, str_help, strlen(str_help));
        } else if (!strncmp(input, "cd", 2)) {
            if (strlen(input) > 2) {
                char *arg = input + 3;
                if (chdir(arg) < 0) {
                    write(out, str_invaliddir, strlen(str_invaliddir));
                }
            }
        } else if (!strncmp(input, "rm", 2)) {
            if (strlen(input) > 2) {
                char *arg = input + 3;
                if (unlink(arg) < 0) {
                    write(out, str_invalidfile, strlen(str_invalidfile));
                }
            }
        } else if (!strncmp(input, "mkdir", 5)) {
            if (strlen(input) > 5) {
                char *arg = input + 6;
                if (mkdir(arg) < 0) {
                    write(out, str_invaliddir, strlen(str_invaliddir));
                }
            }
        } else if (!strncmp(input, "touch", 5)) {
            if (strlen(input) > 5) {
                int fd; 
                char *arg = input + 6;
                fd = open(arg, O_CREAT|O_TRUNC|O_EXCL);
                if (fd < 0) {
                    write(out, str_invalidfile, strlen(str_invalidfile));
                } else close(fd);
            }
        } else if (!strncmp(input, "echo", 4)) {
            if (strlen(input) > 4) {
                int fd; 
                char *arg = input + 5;
                write(out, arg, strlen(arg));
                write(out, "\n", 1);
            }
        } else if (!strncmp(input, "cat", 3)) {
            if (strlen(input) > 3) {
                char *arg = input + 4;
                int fd; 
                fd = open(arg, O_RDONLY);
                if (fd < 0) {
                    write(out, str_invalidfile, strlen(str_invalidfile));
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
            write(out, str_unknowncmd, strlen(str_unknowncmd));
        }
        if (ser != out)
            close(out);
        out = ser;

    }
    close(ser);
}
