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
#include "ioctl.h"
#include "l3gd20_ioctl.h"
#include "lsm303dlhc_ioctl.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <poll.h>
#include <locale.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#ifndef STDIN_FILENO
#   define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#   define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#   define STDERR_FILENO 2
#endif

#define BUFSIZE 256
#define MAXFILES 13

#ifdef LM3S
#  define ARCH	"LM3S"
#elif defined LPC17XX
#  define ARCH	"LPC17XX"
#elif defined STM32F4
#  define ARCH	"STM32F4"
#endif

inline int nargs( void** argv ){
    int argc = 0;
    while( argv[argc] ) argc++;
    return argc;
}


int inputline(char *input, int size){
    int len;
    while(1<2){
        len = 0;
        int out = STDOUT_FILENO;
        int i;
        memset( input, 0, size);
        while( len < size ){
            const char del = 0x08;
            int ret = read( STDIN_FILENO, input + len , 4);
            /*if ( ret > 3 )
                continue;*/
            if ((ret > 0) && (input[len] >= 0x20 && input[len] <= 0x7e)) {
                for (i = 0; i < ret; i++) {
                /* Echo to terminal */
                    if (input[len + i] >= 0x20 && input[len + i] <= 0x7e)
                        write(STDOUT_FILENO, &input[len + i], 1);
                    len++;
                }
            }
            else if( input[len] == 0x0D ){
                input[len + 1] = '\n';
                input[len + 2] = '\0';
                printf("\r\n");
                return len + 2;
            }
            else if( input[len] == 0x4 ){
                if( len != 0 )
                    input[len] = '\0';
                return len;
            }
            /* backspace */
            else if (input[len] == 0x7F || input[len] == 0x08 ) {
                if (len > 0) {
                    write(out, &del, 1);
                    printf( " ");
                    write(out, &del, 1);
                    len--;
                }
            }
        }
        printf("\r\n");
        if (len < 0)
            return -1;

        input[len + 1] = '\0';
    }
    return len;
}


int parse_interval(char* arg, int* start, int* end){
    char *endptr;
    if( *arg == '-' || *arg == ',' ){
        *start = 0;
        endptr = arg;
    }
    else{
        *start = strtol(arg, &endptr, 10);
        if( *start <= 0 && arg != endptr )
            return 1;
        if( *endptr != '-' && *endptr !=',' && *endptr != '\0')
            return 1;
    }
    if( *endptr == '\0' ){
        *end = *start;
        return 0;
    }
    else
        endptr++;
    if( *endptr == '\0' )
        *end = 0;
    else{
        *end = strtol( endptr, &endptr, 10 );
        if( *end == 0 )
            return 1;
    }
    return 0;
}


int bin_ls(void **args)
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
    exit(0);
}


int bin_ln(void **args)
{
    char *file = args[1];
    char *symlink = args[2];

    if (link(file, symlink) < 0) {
        printf("File not found.\r\n");
        exit(-1);
    }
    exit(0);
}

int bin_rm(void **args)
{
    char *file = args[1];

    if (unlink(file) < 0) {
        printf("File not found.\r\n");
        exit(-1);
    }
    exit(0);
}

int bin_mkdir(void **args)
{
    char *file = args[1];

    if (mkdir(file, O_RDWR) < 0) {
        printf("Cannot create directory.\r\n");
        exit(-1);
    }
    exit(0);
}

int bin_touch(void **args)
{
    char *file = args[1];
    int fd;
    fd = open(file, O_CREAT|O_TRUNC|O_EXCL|O_WRONLY);
    if (fd < 0) {
        printf("Cannot create file.\r\n");
        exit(-1);
    } else close(fd);
    exit(0);
}

int bin_echo(void **args)
{
    int i = 1;
    while (args[i]) {
       write(STDOUT_FILENO, args[i], strlen(args[i]));
       write(STDOUT_FILENO, "\r\n", 2);
       i++;
    }
    exit(0);
}

int bin_cat(void **args)
{
    int fd;
    int i = 1;
    while (args[i]) {
        fd = open(args[1], O_RDONLY);
        if (fd < 0) {
            printf("File not found.\r\n");
            exit(-1);
        } else {
            int r;
            char buf[10];
            do {
                r = read(fd, buf, 10);
                if (r > 0) {
                    write(STDOUT_FILENO, buf, r);
                }
            } while (r > 0);
            close(fd);
        }
       i++;
    }
    exit(0);
}

uint32_t rand(void)
{
	int rngfd;
	rngfd = open("/dev/random", O_RDONLY);
	char buf[10];
	int ret = read(rngfd, buf, 1);
	close(rngfd);
	return *((uint32_t *) buf);
}

/*
 * Rolls a dice for each of two players.
 *
 * Returns:		1 if player 1 wins.
 *			-1 if player 2 wins.
 *			0 if it is a tie.
 */
int roll(void)
{
	uint8_t player1 = rand() % 6;
	uint8_t player2 = rand() % 6;
	if (player1 > player2) {
		return 1;
	} else if (player1 < player2) {
		return -1;
	}
	return 0;
}

int bin_dice(void **args)
{
 	char c;
 	int noes = 2;
 	int yes = 0;
 	int user = 0, mcu = 0;

 	printf("\r\nRoll the dice - try out your luck!\r\n\r\n\r\n");
 	while (1) {
 		if (!yes) {
 			printf("Roll?  (Y/n) ");
 			//c = getchar();
 			read(0, &c, 1);
 		} else {
 			yes = 0;
 			c = '\n';
 		}
 		printf("\r");
		if (tolower(c) != 'n') {
			int ret = roll();

			if (ret > 0) {
				user++;
				printf("User won!");
			} else if (ret < 0) {
				mcu++;
				printf("Mcu won!");
			} else {
				printf("It's a tie!");
			}

			printf(" \tstats:  user: \t%d  -  mcu: \t%d\r\n\r\n", user, mcu);

		} else {
			if (noes > 0) {
				printf("Naw, that's really a yes ain't it??\r\n");
				noes--;
				yes++;
			} else {
				break;
			}
		}
		//if ( c != '\n') {
		//	read(0, &c, 1);
		//}
 	}

 	printf("\r\n\r\nRoll the dice is finished! Final score:\r\n\r\n User: \t%d\r\n Mcu: \t%d\r\n\r\nVICTORY FOR ", user, mcu);
 	if (user > mcu) {
 		printf("USER!!!!\r\n");
 	} else if (user < mcu) {
 		printf("MCU!!!!\r\n");
 	} else {
 		printf("NO ONE - it's a TIE!!!!\r\n");
 	}

 	printf("\r\nThank you, come again!\r\n\r\n");
 	exit(0);
}

int bin_random(void **args)
{
	printf("\r\nHere's a random number for ya: \t%u\r\n\r\n", rand());
	exit(0);
}


int bin_dirname( void** args ){
    int argc = 0, i, c;
    extern int optind;
    char delim = '\n';
    char *head, *tail;
    char **flags;
    argc = nargs( args);

    setlocale (LC_ALL, "");

    if ( argc < 2 || args[1] == NULL){
        fprintf(stderr, "usage: dirname [OPTION] NAME...\n");
        exit(1);
    }

    while( (c=getopt(argc,(char**) args, "r") ) != -1 ){
        switch (c){
            case 'r':   /*use NUL char as delimiter instead of \n*/
                delim = (char) 0;
                break;
            default:
                fprintf( stderr, "dirname: invalid option -- '%c'\n", (char)c);
                exit(1);
        }
    }

    i = optind;
    while( args[i] != NULL ){
        head = tail = args[i];
        while (*tail)
            tail++;

    /* removing the last part of the path*/
        while (tail > head && tail[-1] == '/')
            tail--;
        while (tail > head && tail[-1] != '/')
            tail--;
        while (tail > head && tail[-1] == '/')
            tail--;

        if (head == tail)
            printf(*head == '/' ? "/%c" : ".%c", delim);
        else{
            *tail = '\0';
       // printf("%.*s\n", (tail - head), head);
            printf("%s%c", head, delim);
        }
        i++;
    }
    /*resetting getopt*/
    optind = 0;
    exit(0);
}


int bin_tee(void** args)
{
    extern int opterr, optind, optopt;
    int c, i, argc = 0, written, j = 0,  b = 0;
    char line[BUFSIZE];
    int slot, fdfn[MAXFILES + 1][2] ;
    ssize_t n, count;
    int mode = O_WRONLY | O_CREAT | O_TRUNC;
    argc = nargs( args );
    setlocale(LC_ALL, "");
    opterr = 0;
    optind = 0;
    fdfn[j][0] = STDOUT_FILENO;
    while ((c = getopt(argc,(char**) args, "ai")) != -1)
        switch (c)
        {
        case 'a':   /* append to rather than overwrite file(s) */
            mode = O_WRONLY | O_CREAT | O_APPEND;
            break;
        case 'i':   /* ignore the SIGINT signal */
            signal(SIGINT, SIG_IGN);
            break;
        case 0:
            opterr = 0;
            break;
        default:
            fprintf(stderr, "tee: invalid option -- '%c'\n", c,optopt);
            exit(1);
        }
    /*setting extra stdout redirections, getopt does not count them*/
    for( i = 0; i < optind ; i++ ){
        if( strcmp( args[i] , "-" ) == 0 )
            fdfn[++j][0] = STDOUT_FILENO;
    }

    for (slot = j+1; i < argc ; i++)
    {
        if (slot > MAXFILES)
            fprintf(stderr, "tee: Maximum of %d output files exceeded\n", MAXFILES);
        else
        {
            if ((fdfn[slot][0] = open(args[i], mode, 0666)) == -1)
                fprintf(stderr,"error opening %s\n", args[i]);
            else
                fdfn[slot++][1] = i;
        }
    }
    i = 0;
    while( inputline(line, BUFSIZE) != 0 ){
        for( i = 0; i < slot; i++ ){
            count = strlen( line );
            while( count > 0 ){
                written = write( fdfn[i][0], line, count);
                count -= written;
            }
        }
    }

    if (n < 0)
        printf("error\n");

    for (i = 1; i < slot; i++)
        if (close(fdfn[i][0]) == -1)
            printf("error\n");
    optind = 0;
    exit(0);
}


int bin_true(void **args)
{
	exit(0);
}

int bin_false(void **args)
{
	exit(1);
}

int bin_arch(void **args)
{
	printf("%s\r\n", ARCH);
	exit(0);
}

int bin_mem_fault(void **args) {
    uint32_t *faulty = (uint32_t *)0x40000100;
    printf("%d\r\n", *faulty);
}

int bin_kmem_fault(void **args) {
    uint32_t *faulty = (uint32_t *)0x20000100;
    printf("%d\r\n", *faulty);
}

int bin_wc(void **args)
{
    int fd;
    int i = 1;
    while (args[i]) {
    	int last_white = 0;
    	int bytes = 0, words = 0, newlines = 0;
        fd = open(args[i], O_RDONLY);
        if (fd < 0) {
            printf("File not found.\r\n");
            exit(-1);
        } else {
            int r;
            char buf[1];
            do {
                r = read(fd, buf, 1);
                if (r > 0) {
                    bytes += r;
                    if (buf[0] == '\n') {
                        newlines++;
                	words++;
                	last_white = 1;
                    } else if ((buf[0] == ' ') || (buf[0] == '\t')) {
                	words++;
                	last_white = 1;
                    } else {
                    	last_white = 0;
                    }
                }
            } while (r > 0);
            close(fd);
        }
        if (!last_white) {
        	words++;
        	newlines++;
        }
    	printf("%d %d %d %s\r\n", newlines, words, bytes, args[i]);
    	i++;
    }
    exit(0);
}

#define L3GD20_WHOAMI              0x0F
#define L3GD20_CTRL_REG1      0x20
#define L3GD20_CTRL_REG2       0x21
#define L3GD20_CTRL_REG3       0x22
#define L3GD20_CTRL_REG4       0x23
#define L3GD20_CTRL_REG5       0x24
#define L3GD20_REFERENCE       0x25
#define L3GD20_OUT_TEMP        0x26
#define L3GD20_STATUS_REG      0x27
#define L3GD20_OUT_X_L         0x28
#define L3GD20_OUT_X_H         0x29
#define L3GD20_OUT_Y_L         0x2A
#define L3GD20_OUT_Y_H         0x2B
#define L3GD20_OUT_Z_L         0x2C
#define L3GD20_OUT_Z_H         0x2D
#define L3GD20_FIFO_CTRL_REG 0x2E
#define L3GD20_FIFO_SRC_REG    0x2F
#define L3GD20_INT1_CFG        0x30
#define L3GD20_INT1_SRC        0x31
#define L3GD20_INT1_TSH_XH 0x32
#define L3GD20_INT1_TSH_XL 0x33
#define L3GD20_INT1_TSH_YH 0x34
#define L3GD20_INT1_TSH_YL 0x35
#define L3GD20_INT1_TSH_ZH 0x36
#define L3GD20_INT1_TSH_ZL 0x37
#define L3GD20_INT1_DURATION 0x38

int bin_gyro(void **args)
{
    int fd, i=100;
    unsigned char buffer[6];
    struct l3gd20_ctrl_reg l3gd20;

    if(nargs(args) != 2 || args[1] == NULL)
    {
        printf("Usage : gyro on|off\n\r");
        goto exit;
    }

    fd = open("/dev/l3gd20", O_RDONLY);
    if (fd < 0) {
        printf("File not found.\r\n");
        exit(-1);
    }

    if(strcasecmp(args[1], "off") == 0)
    {
        l3gd20.reg = L3GD20_CTRL_REG1;
        l3gd20.data = 0x00;
        ioctl(fd, IOCTL_L3GD20_WRITE_CTRL_REG, &l3gd20);
    }
    else
    {
        l3gd20.reg = L3GD20_WHOAMI;
        ioctl(fd, IOCTL_L3GD20_READ_CTRL_REG, &l3gd20);
        printf("WHOAMI=%02X\n\r",l3gd20.data);

        l3gd20.reg = L3GD20_CTRL_REG1;
        l3gd20.data = 0x0F;                 /*PD | Zen | Yen | Zen  */
        ioctl(fd, IOCTL_L3GD20_WRITE_CTRL_REG, &l3gd20);

        l3gd20.reg = L3GD20_CTRL_REG3;
        l3gd20.data = 0x08;
        ioctl(fd, IOCTL_L3GD20_WRITE_CTRL_REG, &l3gd20);

        while(i--) {
            read(fd, buffer ,6);
            printf("%04X %04X %04X\n\r", *((uint16_t*)buffer), *((uint16_t*)&buffer[2]), *((uint16_t*)&buffer[4]) ); 
        }
    }
    close(fd);

exit:
    exit(0);
}

#define LSM303ACC_CTRL_REG1      0x20

int bin_acc(void **args)
{
    int fd;
    struct lsm303dlhc_ctrl_reg lsm303dlhc;

    if(nargs(args) != 2 || args[1] == NULL)
    {
        printf("Usage : acc on|off\n\r");
        goto exit;
    }

    fd = open("/dev/lsm303acc", O_RDONLY);
    if (fd < 0) {
        printf("File not found.\r\n");
        exit(-1);
    }

    if(strcasecmp(args[1], "off") == 0)
    {
        lsm303dlhc.reg = LSM303ACC_CTRL_REG1;
        lsm303dlhc.data = 0x00;
        ioctl(fd, IOCTL_LSM303DLHC_WRITE_CTRL_REG, &lsm303dlhc);
    }
    else
    {
        lsm303dlhc.reg = LSM303ACC_CTRL_REG1;
        lsm303dlhc.data = 0x0F;                 /*PD | Zen | Yen | Zen  */
        ioctl(fd, IOCTL_LSM303DLHC_WRITE_CTRL_REG, &lsm303dlhc);

        lsm303dlhc.reg = LSM303ACC_CTRL_REG1;
        lsm303dlhc.data = 0x00;                 
        ioctl(fd, IOCTL_LSM303DLHC_READ_CTRL_REG, &lsm303dlhc);

    }
    close(fd);

exit:
    exit(0);
}

#define CRA_REG_M   0x00

int bin_mag(void **args)
{
    int fd;
    struct lsm303dlhc_ctrl_reg lsm303dlhc;

    if(nargs(args) != 2 || args[1] == NULL)
    {
        printf("Usage : mag on|off\n\r");
        goto exit;
    }

    fd = open("/dev/lsm303mag", O_RDONLY);
    if (fd < 0) {
        printf("File not found.\r\n");
        exit(-1);
    }

    if(strcasecmp(args[1], "off") == 0)
    {
        lsm303dlhc.reg = CRA_REG_M;
        lsm303dlhc.data = 0x00;
        ioctl(fd, IOCTL_LSM303DLHC_WRITE_CTRL_REG, &lsm303dlhc);
    }
    else
    {
        lsm303dlhc.reg = CRA_REG_M;
        lsm303dlhc.data = 0x10;
        ioctl(fd, IOCTL_LSM303DLHC_WRITE_CTRL_REG, &lsm303dlhc);

        lsm303dlhc.reg = CRA_REG_M;
        lsm303dlhc.data = 0x00;                 
        ioctl(fd, IOCTL_LSM303DLHC_READ_CTRL_REG, &lsm303dlhc);

    }
    close(fd);

exit:
    exit(0);
}



int bin_realloc(void **args)
{
    char * ptr;
    ptr = malloc(2);
    ptr = realloc(ptr, 4);
    free(ptr);
}


/*returns   1 if it reaches a newline,
            0 if a delimiter is found,
            -1 if the file is finished*/
int readuntil( int fd, char** line, char delim){
    int b, len;
    len = -1;
    do{
        len++;
        b = read( fd, (*line) + len, 1 );
    }while( (*line)[len] != delim && (*line)[len] != '\n' && b > 0 );
    (*line)[len+1] = '\0';
    if( b <= 0 )
        return -1;
    if( (*line)[len] == '\n')
        return 1;
    if( (*line)[len] == delim )
        return 0;
}



int bin_cut( void** args){
    extern int opterr, optind;
    extern char* optarg;
    char *endptr, *line;
    int c, start, end, i, j, len, flags, slot, argc;
    int b = 0;
    int mode = O_RDONLY;
    opterr = 0;
    optind = 0;
    j = 0;
    char delim = '\t';
    argc = nargs(args);
    int fdfn[MAXFILES];
    while( (c = getopt( argc, (char**)args, "d:c:f:" )) != -1){
        switch (c){
            case 'f':
            case 'c':
                if( b == 1 ){
                    fprintf(stderr,"cut: only one type of list may be specified\n");
                    exit(1);
                }
                b = 1;
                flags = c;
                if( parse_interval(optarg, &start, &end)!=0){
                    fprintf(stderr,"cut: invalid interval\n");
                    exit(1);
                }
                break;
            case 'd':
                len = strlen( optarg );
                if( len <= 0 ){
                    fprintf(stderr, "cut: please specify a valid delimiter\n");
                    exit(1);
                }
                if( ( optarg[0] == '"' && optarg[2] == '"' ) || ( optarg[0] == '\'' && optarg[2] == '\'' ) )
                    delim = optarg[1];
                else
                    delim = optarg[0];
                break;
            default:
                fprintf(stderr,"cut: invalid option -- '%c'\n", optopt );
                exit(1);
        }
    }
    if( b == 0 ){
        fprintf(stderr,"cut: you must specify at list of characters\n");
        exit(1);
    }
    if( delim != '\t' && flags != 'f' ){
        fprintf(stderr, "cut: an input delimiter may be specified only when operating on fields\n");
        exit(1);
    }
    if(--start < 0 )
        start = 0;
    end--;
    line = (char*) malloc( sizeof(char)*BUFSIZE );
    if( args[optind] ){
        slot = 0;
        for( i = optind; i < argc; i++ ){
            if( i > MAXFILES )
                fprintf(stderr, "cut: Maximum of %d output files exceeded\n", MAXFILES);
            if(( fdfn[slot] = open( args[i], mode, 0666)) == -1 )
                fprintf(stderr, "error opening %s\n", args[i] );
            else
                slot++;
        }
        if( flags == 'c' ){
            len = (end > 0) ? end - start : BUFSIZE - start;
            for( i = 0; i < slot; i++ ){
                do{
                    for( j = 0; j < start; j++ ){
                        b = read(fdfn[i], line + j, 1 );
                        if( b <= 0 || line[j] == '\n')
                            break;
                    }
                    if( b <= 0 || j != start ){
                        printf("\n");
                        continue;
                    }
                    for( j = 0; j <= len; j++ ){
                        b = read( fdfn[i], line + j, 1 );
                        if( b <= 0 || line[j] == '\n' ){
                            j++;
                            break;
                        }
                    }
                    if( b <= 0 )
                        break;
                    line[j] = '\0';
                    if( line[j-1] != '\n' )
                        printf( "%s\n", line);
                    else
                        printf( "%s", line );
                    line[0] = line[j-1];
                    while( line[0] != '\n' )
                        if( read( fdfn[i], line, 1 ) <= 0 )
                            break;
                }while( b > 0 );
            }
        }
        else if( flags == 'f' ){
            for( i = 0; i < slot; i++ ){
                while( b >= 0 ){
                    if( start == 0 )
                        b = 0;
                    for( j = 0; j < start ; j++ ){
                        if ((b = readuntil( fdfn[i], &line, delim ) ) != 0)
                            break;
                    }
                    if( b < 0 )
                        break;
                    else if( b == 1 ){
                        printf("%s", line);
                        continue;
                    }
                    /*eventually the for will break if end < 0*/
                    for( ;  end < 0  || j <= end ; j++){
                        b = readuntil( fdfn[i], &line, delim );
                        if( b != 0 ){
                            if( b == 1 )
                                printf("%s", line);
                            break;
                        }
                        if( j == end && end > 0 )
                            line[ strlen(line) - 1 ] = '\0';
                        printf( "%s", line );
                    }
                    if( b < 0 )
                        break;
                    else if( b == 0 ){
                        write( STDOUT_FILENO, "\r\n", 2 );
                        readuntil( fdfn[i], &line, '\n' );
                    }
                }
            }
        }
    }
    else{
        /*stdin*/
        while( inputline(line, BUFSIZE) != 0 ){
            if( flags == 'c' ){
                len = strlen(line)-1;
                len = (end < len && end >= 0) ? end : len - 1;
                for( i = start; i <= len; i++ )
                    write( STDOUT_FILENO, &line[i], 1 );
                write( STDOUT_FILENO, "\r\n", 2 );
            }
            else if( flags == 'f' ){
                len = strlen(line);
                j = 0;
                for( i = 0; i < start ; i++){
                    while( line[j] != delim && j < len ) j++;
                    if( j < len ) j++;
                }
                if( j == len ){
                    printf("%s", line );
                    continue;
                }
                for( i = start; end < 0 || i <= end; i++){
                    while( line[j] != delim && j < len )
                        write( STDOUT_FILENO, &line[j++], 1 );
                    if( line[j] == delim && (i < end || end < 0) )
                        write( STDOUT_FILENO, &line[j++], 1 );
                    if( j == len )
                        break;
                }
                if( line[j-1] != '\n')
                    write( STDOUT_FILENO, "\r\n", 2 );
            }
        }
    }
    exit(0);
}


int dits(int led)
{
    write(led, "1", 1);
    sleep(100);
    write(led, "0", 1);
    sleep(100);
    return 0;
}

int dahs(int led)
{
    write(led, "1", 1);
    sleep(300);
    write(led, "0", 1);
    sleep(100);
    return 0;
}

int bin_morse(void **args)
{
#ifdef PYBOARD
# define LED0 "/dev/gpio_1_13"
#elif defined (STM32F4)
# if defined (F429DISCO)
#  define LED0 "/dev/gpio_6_13"
# else
#  define LED0 "/dev/gpio_3_12"
#endif
#elif defined (LPC17XX)
#if 0
/*LPCXpresso 1769 */
# define LED0 "/dev/gpio_0_22"
#else
/* mbed 1768 */
# define LED0 "/dev/gpio_1_18"
#endif
#else
# define LED0 "/dev/null"
#endif

	int led = open(LED0, O_RDWR, 0);

	char name[128] = "anti ANTI anti ANTI 0123456789";
	if (!args[1]) {
		memcpy(args[1], name, 128);
	}
    const uint8_t morse[][6] = 	{{1, 2, 0},		// a
				{2, 1, 1, 1, 0},	// b
				{2, 1, 2, 1, 0},	// c
				{2, 1, 1, 0},		// d
				{1, 0},			// e
				{1, 1, 2, 1, 0},	// f
				{2, 2, 1, 0},		// g
				{1, 1, 1, 1, 0},	// h
				{1, 1, 0},		// i
				{1, 2, 2, 2, 0},	// j
				{2, 1, 2, 0},		// k
				{1, 2, 1, 1, 0},	// l
				{2, 2, 0},		// m
				{2, 1, 0},		// n
				{2, 2, 2, 0},		// o
				{1, 2, 2, 1, 0},	// p
				{2, 2, 1, 2, 0},	// q
				{1, 2, 1, 0},		// r
				{1, 1, 1, 0},		// s
				{2, 0},			// t
				{1, 1, 2, 0},		// u
				{1, 1, 1, 2, 0},	// v
				{1, 2, 2, 0},		// w
				{2, 1, 1, 2, 0},	// x
				{2, 1, 2, 2, 0},	// y
				{2, 2, 1, 1, 0},	// z
				{2, 2, 2, 2, 2, 0},	// 0
				{1, 2, 2, 2, 2, 0},	// 1
				{1, 1, 2, 2, 2, 0},	// 2
				{1, 1, 1, 2, 2, 0},	// 3
				{1, 1, 1, 1, 2, 0},	// 4
				{1, 1, 1, 1, 1, 0},	// 5
				{2, 1, 1, 1, 1, 0},	// 6
				{2, 2, 1, 1, 1, 0},	// 7
				{2, 2, 2, 1, 1, 0},	// 8
				{2, 2, 2, 2, 1, 0},	// 9
			};
	int i = 0, j = 0, dec = 0x42, k = 1;
	do {
		if (args[k]) {
			memcpy(name, args[k], 128);
		}
		for (i = 0; i < strlen(name); i++) {
			while (1) {
				if (name[i] == ' ') {
					sleep(200);
					break;
				} else if (name[i] >= 'a' && name[i] <= 'z') {
					dec = 'a';
				} else if (name[i] >= 'A' && name[i] <= 'Z') {
					dec = 'A';
				} else if (name[i] >= '0' && name[i] <= '9') {
					dec = 0x30;
				}
				if (morse[(name[i] - dec)][j] == 1) {
					dits(led);
					j++;
				} else if (morse[(name[i] - dec)][j] == 2) {
					dahs(led);
					j++;
				} else if (morse[(name[i] - dec)][j] == 0) {
					sleep(200);
					j = 0;
					break;
				}
			}
		}
		sleep(200);
		k++;
	} while (args[k]);
	close(led);
	exit(0);
}

struct cm_board {
	uint8_t wolf;
	uint8_t sheep;
};

int cm_init_board(struct cm_board *b)
{
	b->wolf = rand() % 256;
	b->sheep = rand() % 256;
	//printf("Wolf is at :  %02X  -  sheep is at :  %02X\r\n", b->wolf, b->sheep);
	return 0;
}

uint8_t cm_move(struct cm_board *b)
{
	char input[8];
	int ret;
	uint8_t steps =1;

	ret = read(STDIN_FILENO, input, 3);
	if ((ret == 3) && (input[0] == 0x1b)) {
		//printf("GOT: %02X %02X %c\r\n", input[0], input[1], input[2]);
		if (input[2] == 'A') {					// UP
			steps = (((b->wolf & 0xF) + 1) & 0xF);
			b->wolf = (b->wolf & 0xF0) + steps;
		} else if (input[2] == 'B') {				// DOWN
			steps = (((b->wolf & 0xF) - 1) & 0xF);
			b->wolf = (b->wolf & 0xF0) + steps;
		} else if (input[2] == 'C') {				// RIGHT
			steps = (((b->wolf & 0xF0) + 0x1F) & 0xF0);
			b->wolf = (b->wolf & 0x0F) + steps;
		} else if (input[2] == 'D') {				// LEFT
			steps = (((b->wolf & 0xF0) - 1) & 0xF0);
			b->wolf = (b->wolf & 0x0F) + steps;
		}
		//printf("Wolf is at :  %02X  -  sheep is at :  %02X\r\n", b->wolf, b->sheep);
	} else if ((ret == 1) && (input[0] == 'q')) {
		return 0;
	}
	return steps;
}

void cm_indicator(struct cm_board *b, int led)
{
	int delay, i;

	uint8_t diff = (b->wolf & 0xF) - (b->sheep & 0xF);

	if (diff > 0xF) {
		diff = 0xFF - diff;
	}

	uint8_t diff2 = ((((b->wolf >> 4) & 0xF) - ((b->sheep >> 4) & 0xF)));
	if (diff2 > 0xF) {
		diff2 = 0xFF - diff2;
	}

	diff += diff2;
	diff == diff / 2;
	delay = diff * 50;

	for (i = 0; i < 5; i++) {
		write(led, "1", 1);
		sleep(delay);
		write(led, "0", 1);
		sleep(delay);
	}
}

void cm_disco(int led)
{
	int i;
	for (i = 0; i < 10; i++) {
		write(led, "1", 1);
		sleep(50);
		write(led, "0", 1);
		sleep(30);
		write(led, "1", 1);
		sleep(100);
		write(led, "0", 1);
		sleep(30);
	}
}

int bin_catch_me(void **args)
{
	int led = open(LED0, O_RDWR, 0);
	struct cm_board *b = malloc(sizeof(struct cm_board));

	cm_init_board(b);

	printf("You're a hungry wolf. A sheep is hiding on this 16*16 board..\r\n");
	printf("Use the arrows and let the blinky led guide you...\r\n");

	while (1) {
		int dir = cm_move(b);
		if (dir) {
			if (b->wolf == b->sheep) {
				printf("You won!\r\n");
				cm_disco(led);
				break;
			}
			cm_indicator(b, led);
		} else {
			printf("Thought so.\r\n");
			break;
		}
	}
	close(led);
	exit(0);
}

int bin_kill(void **args)
{
    int pid;
    if ((args[1] == NULL) || (args[2] != NULL)) {
        printf("Usage: %s pid\r\n", args[0]);
        exit(1);
    }
    pid = atoi(args[1]);
    if (pid < 1) {
        printf("Usage: %s pid\r\n", args[0]);
        exit(2);
    }
    if (pid == 1) {
        printf("Error: Can't kill init!\r\n");
        exit(3);
    }

    kill(pid, SIGTERM);
    exit(0);
}

int bin_test_realloc(void **args)
{
    char **ptr;
    ptr=malloc(sizeof(char*));
    printf("PTR: %08X\r\n", ptr);
    ptr=realloc( ptr, (2 * sizeof(char*)));
    printf("PTR: %08X\r\n", ptr);
    free(ptr);
    exit(0);

}

int bin_test_doublefree(void **args)
{
    char **ptr;
    ptr=malloc(sizeof(char*));
    printf("PTR: %08X\r\n", ptr);
    free(ptr);
    free(ptr);
    exit(0);
}
