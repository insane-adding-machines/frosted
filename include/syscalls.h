/* Frosted system call API */
#include <stdio.h>

struct stat;
struct tms;
 
void _exit();
int fork(void);
int fstat(int fildes, struct stat *st);
int isatty(int file);
int stat(const char *file, struct stat *st);
int link(char *existing, char *new);
int close(int file);
int getpid();
int kill(int pid, int sig);
int lseek(int file, int ptr, int dir);
int open(const char *name, int flags, ...);
int read(int file, void *ptr, int len);
int unlink(char *name);
int wait(int *status);
int write(int file, const void *ptr, int len);
void free(void * ptr);
void * malloc(int size);

