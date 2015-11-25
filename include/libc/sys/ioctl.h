/* libc/sys/linux/sys/ioctl.h - ioctl prototype */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

/* #include <bits/ioctls.h> */

int ioctl(int fd,int request,...);
 	
struct winsize
{
  unsigned short ws_row;	/* rows, in characters */
  unsigned short ws_col;	/* columns, in characters */
  unsigned short ws_xpixel;	/* horizontal size, pixels */
  unsigned short ws_ypixel;	/* vertical size, pixels */
};

#define TIOCGWINSZ	0x5413

#endif
