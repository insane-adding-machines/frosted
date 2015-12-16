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
#include <poll.h>

#ifndef STDIN_FILENO
#   define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#   define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#   define STDERR_FILENO 2
#endif


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

/*
 * Rolls a dice for each of two players.
 *
 * Returns:		1 if player 1 wins.
 *			-1 if player 2 wins.
 *			0 if it is a tie.
 */
int roll(void)
{
	int rngfd;
	rngfd = open("/dev/random", O_RDONLY);
	char buf[10];
	int ret = read(rngfd, buf, 1);//rand() % 6; // we'll replace this with a read to /dev/random, and later with the frosted rand() function.
	//int player2 = //rand() %6;
	int player1 = *((uint32_t *) buf);
	player1 = player1 % 6;
	ret = read(rngfd, buf, 1);//rand() % 6; // we'll replace this with a read to /dev/random, and later with the frosted rand() function.
	int player2 = *((uint32_t *) buf);
	player2 = player2 % 6;
	if (player1 > player2) {
		return 1;
	} else if (player1 < player2) {
		return -1;
	}
	return 0;
}

int dice(void)
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
		if ( c != '\n') {
			read(0, &c, 1);
		}
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
