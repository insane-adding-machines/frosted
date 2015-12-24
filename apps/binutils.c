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

inline int nargs( void** argv ){
    int argc = 0;
    while( argv[argc] ) argc++;
    return argc;
}


char *readln(char *input, int size){
    while(1<2){
        int len = 0;
        int out = STDOUT_FILENO;
        int i;
        memset( input, 0, size);
        while( len < size ){
            const char del = 0x08;
            int ret = read( STDIN_FILENO, input + len , 3);
            if ( ret > 3 )
                continue;
            if ((ret > 0) && (input[len] >= 0x20 && input[len] <= 0x7e)) {
                for (i = 0; i < ret; i++) {
                /* Echo to terminal */
                    if (input[len + i] >= 0x20 && input[len + i] <= 0x7e)
                        write(STDOUT_FILENO, &input[len + i], 1);
                    len++;
                }
            }
            if( input[len] == 0x0D ){
                input[len + 1] = '\n';
                input[len + 2] = '\0';
                printf("\r\n");
                if( len == 0 ) return NULL;  
                return input;
            }
            if( input[len] == 0x4 ){
                printf("\r\n");
                len = 0;
                break;
            }
            /* backspace */
            if ((input[len] == 127)) {
                if (len > 1) {
                    write(out, &del, 1);
                    printf( " ");
                    write(out, &del, 1);
                    len -= 2;
                }else {
                    len -=1;
                }
            }
        }
        printf("\r\n");
        if (len < 0)
            return NULL;

        input[len + 1] = '\0';
    }
    return input;       
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
    fd = open(file, O_CREAT|O_TRUNC|O_EXCL);
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

int bin_dice(void)
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

int bin_random(void)
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
    extern int opterr, optind;
    int c, i, argc = 0, written, j = 0,  b = 0;
    char line[BUFSIZE];
    int slot, fdfn[MAXFILES + 1][2] ;
    ssize_t n, count;
    int mode = O_WRONLY | O_CREAT | O_TRUNC;
    argc = nargs( args );
    setlocale(LC_ALL, "");
    opterr = 0;
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
            fprintf(stderr, "tee: invalid option -- '%c'\n", (char)c);
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
                printf("error\n");
            else
                fdfn[slot++][1] = i;
        }
    }
    i = 0;
    while( 1 ){
        if (readln( line, BUFSIZE) == NULL ){
            if(b) break;
            b = 1;
        }
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

    exit(0);
}
