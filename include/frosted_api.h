#ifndef INC_FROSTED_API
#define INC_FROSTED_API
#include <stdint.h>

#define INIT __attribute__((section(".init")))

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

/* semaphore */
struct semaphore;
typedef struct semaphore sem_t;
typedef struct semaphore mutex_t;

#define MAX_FILE 64
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
#define P_IFMT     0000007   // bit mask for file permissions

#define S_IFSOCK   0140000   // socket
#define S_IFLNK    0120000   // symbolic link
#define S_IFREG    0100000   // regular file
#define S_IFBLK    0060000   // block device
#define S_IFDIR    0040000   // directory
#define S_IFCHR    0020000   // character device
#define S_IFIFO    0010000   // FIFO

#define P_EXEC     0000001   // exec

#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)    // is it a regular file?
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)    // directory?
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)    // link?
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)    // character device?

/* poll */
#define POLLIN		0x0001
#define POLLPRI		0x0002
#define POLLOUT		0x0004
#define POLLERR		0x0008
#define POLLHUP		0x0010
#define POLLNVAL	0x0020

struct pollfd {
    int fd;
    uint16_t events;
    uint16_t revents;
};


/* for unix sockets */
#define AF_UNIX 0
#define SOCK_STREAM 6
#define SOCK_DGRAM 17

struct __attribute__((packed)) sockaddr {
    uint16_t sa_family;
    uint8_t  sa_zero[14];
};

struct __attribute__((packed)) sockaddr_un {
    uint16_t sun_family;
    uint8_t  sun_path[MAX_FILE - 2];
};

struct sockaddr_env {
    struct sockaddr *se_addr;
    unsigned int se_len;
};

#endif
