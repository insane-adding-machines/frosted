#ifndef INC_FROSTED_API
#define INC_FROSTED_API
#include <stdint.h>

/* Constants */

/* open */
#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR   (O_RDONLY | O_WRONLY)
#define O_CREAT  0x04
#define O_EXCL   0x08
#define O_TRUNC  0x10
#define O_APPEND 0x20

/* seek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* syslog */
#define LOG_EMERG   0   /* system is unusable */
#define LOG_ALERT   1   /* action must be taken immediately */
#define LOG_CRIT    2   /* critical conditions */
#define LOG_ERR     3   /* error conditions */
#define LOG_WARNING 4   /* warning conditions */
#define LOG_NOTICE  5   /* normal but significant condition */
#define LOG_INFO    6   /* informational */
#define LOG_DEBUG   7   /* debug-level messages */

/* opendir - readdir */
typedef void DIR;

#define MAX_FILE 48
struct dirent {
    uint32_t d_ino;
    char d_name[MAX_FILE];
};

/* stat */
struct stat {
    uint32_t st_size;
    uint32_t st_mode;
    struct module *st_owner;
};

#define S_IFMT     0170000   // bit mask for the file type bit fields

#define S_IFSOCK   0140000   // socket
#define S_IFLNK    0120000   // symbolic link
#define S_IFREG    0100000   // regular file
#define S_IFBLK    0060000   // block device
#define S_IFDIR    0040000   // directory
#define S_IFCHR    0020000   // character device
#define S_IFIFO    0010000   // FIFO

#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)    // is it a regular file?
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)    // directory?
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)    // character device?



/* Syscalls API for frosted. 
 * Any Application may link to the OS using these system calls.
 *
 */
#ifndef KERNEL
int sys_setclock(unsigned int ms);
int sys_sleep(unsigned int ms);
int sys_suspend(unsigned int ms);
int sys_thread_create(void (*init)(void *), void *arg, unsigned int prio);
int sys_mkdir(char *path);
int sys_getpid(void);
int sys_getppid(void);
int sys_open(const char *path, int mode);
int sys_close(int fd);
DIR *sys_opendir(const char *path);
int sys_readdir(DIR *d, struct dirent *ep);
int sys_closedir(DIR *d);
unsigned int sys_gettimeofday(unsigned int *ms);
int sys_stat(const char *path, struct stat *st);

void *sys_malloc(uint32_t size);
int sys_free(void *ptr);
#endif /* KERNEL */

#endif
