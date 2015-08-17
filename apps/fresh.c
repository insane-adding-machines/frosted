#include "frosted_api.h"
#include <string.h>
#include <stdio.h>
static const char str_welcome[]      = "Welcome to Frosted\r\n";
static const char str_unknowncmd[]   = "Unknown command. Try 'help'.\r\n";
static const char str_invaliddir[]   = "Directory not found.\r\n";
static const char str_invalidfile[]  = "File not found.\r\n";
static const char str_help[]         = "Supported commands: help ls mkdir touch rm.\r\n";
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

    fname_start = sys_malloc(MAX_FILE);
    ep = sys_malloc(sizeof(struct dirent));
    if (!ep || !fname_start)
        while(1);;

    d = sys_opendir(start);
    while(sys_readdir(d, ep) == 0) {
        fname = fname_start;
        fname[0] = '\0';
        strncat(fname, start, MAX_FILE);
        strncat(fname, "/", MAX_FILE);
        strncat(fname, ep->d_name, MAX_FILE);

        while (fname[0] == '/' && fname[1] == '/')
            fname++;

        sys_stat(fname, &st);
        sys_write(ser, fname, strlen(fname));
        sys_write(ser, "\t", 1);
        if (S_ISDIR(st.st_mode))
            type = 'd';
        else
            type = 'f';

        sys_write(ser, &type, 1); 
        sys_write(ser, "\n", 1);
    }
    sys_closedir(d);
    sys_free(ep);
    sys_free(fname_start);
}

void fresh(void *arg) {
    int ser;
    char pwd[MAX_FILE] = "";

    do {
        ser = sys_open("/dev/ttyS0", O_RDWR);
    } while (ser < 0);

    sys_write(ser, str_welcome, strlen(str_welcome));
    sys_mkdir("/home");
    sys_mkdir("/home/test");
    sys_chdir("/home/test");

    while (2>1)
    {
        char input[100];
        int len = 0;
        struct pollfd pfd;
        pfd.fd = ser;
        pfd.events = POLLIN;

        sys_write(ser, str_prompt, strlen(str_prompt));
        sys_getcwd(pwd, MAX_FILE);
        sys_write(ser, pwd, strlen(pwd));
        sys_write(ser, "$ ", 2);

        /* Blocking calls: WIP
        if (sys_poll(&pfd, 1, 1000) <= 0)
            continue;
        */

        while(len < 100)
        {
            len += sys_read(ser, input+len, 1);
            if (input[len-1] == 0xD)
                break; /* CR (\n) */
        }
        sys_write(ser, "\n", 1);

        input[len - 1] = '\0';
        if (!strncmp(input, "ls", 2))
        {
            ls(ser, pwd);
        } else if (!strncmp(input, "help", 4)) {
            sys_write(ser, str_help, strlen(str_help));
        } else if (!strncmp(input, "cd", 2)) {
            if (strlen(input) > 2) {
                char *arg = input + 3;
                if (sys_chdir(arg) < 0) {
                    sys_write(ser, str_invaliddir, strlen(str_invaliddir));
                }
            }
        } else if (!strncmp(input, "rm", 2)) {
            if (strlen(input) > 2) {
                char *arg = input + 3;
                if (sys_unlink(arg) < 0) {
                    sys_write(ser, str_invalidfile, strlen(str_invalidfile));
                }
            }
        } else if (!strncmp(input, "mkdir", 5)) {
            if (strlen(input) > 5) {
                char *arg = input + 6;
                if (sys_mkdir(arg) < 0) {
                    sys_write(ser, str_invaliddir, strlen(str_invaliddir));
                }
            }
        } else if (!strncmp(input, "touch", 5)) {
            if (strlen(input) > 5) {
                int fd; 
                char *arg = input + 6;
                fd = sys_open(arg, O_CREAT|O_TRUNC|O_EXCL);
                if (fd < 0) {
                    sys_write(ser, str_invalidfile, strlen(str_invalidfile));
                } else sys_close(fd);
            }
        } else if (strlen(input) > 0){
            sys_write(ser, str_unknowncmd, strlen(str_unknowncmd));
        }
    }
    close(ser);
}
