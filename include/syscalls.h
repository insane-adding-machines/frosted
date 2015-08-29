/* Frosted system call API */
#include <stdio.h>
#ifndef _INCLUDE_SYSCALLS
#define _INCLUDE_SYSCALLS

struct stat;
struct tms;
struct timeval;
struct timezone;
typedef void DIR;



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
void *malloc(int size);
DIR *opendir(const char *path);
int mkdir(char *path);
int readdir(DIR *d, struct dirent *ep);
int closedir(DIR *d);
unsigned int gettimeofday(unsigned int *ms);
#endif
