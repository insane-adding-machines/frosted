#ifndef FRAMEBUFFER_INCLUDED
#define FRAMEBUFFER_INCLUDED

#include <stdint.h>
#include "frosted.h"
#include <sys/frosted-io.h>

struct fb_ops;


struct fb_info {
        struct fb_var_screeninfo var;   /* Current var */
        //struct fb_fix_screeninfo fix;   /* Current fix */
        struct fb_videomode *mode;      /* current mode */
        //struct backlight_device *bl_dev;

        struct fb_ops *fbops;
        struct device *dev;             /* This is this fb device */
        uint8_t *screen_buffer;   /* Framebuffer address */
};

struct fb_ops {
        /* open/release and usage marking */
        int (*fb_open)(struct fb_info *info);
        int (*fb_release)(struct fb_info *info);

        /* checks var and eventually tweaks it to something supported,
         * DO NOT MODIFY PAR */
        int (*fb_check_var)(struct fb_var_screeninfo *var, struct fb_info *info);

        /* set the video mode according to info->var */
        int (*fb_set_par)(struct fb_info *info);

        /* set color registers in batch */
        int (*fb_setcmap)(uint32_t *cmap, struct fb_info *info);

        /* blank display */
        int (*fb_blank)(struct fb_info *info);

        /* Draws a rectangle */
        //void (*fb_fillrect) (struct fb_info *info, const struct fb_fillrect *rect);
        /* Copy data from area to another */
        //void (*fb_copyarea) (struct fb_info *info, const struct fb_copyarea *region);
        /* Draws a image to the display */
        //void (*fb_imageblit) (struct fb_info *info, const struct fb_image *image);

        /* Draws cursor */
        //int (*fb_cursor) (struct fb_info *info, struct fb_cursor *cursor);

        /* Rotates the display */
        //void (*fb_rotate)(struct fb_info *info, int angle);

        /* perform fb specific ioctl (optional) */
        int (*fb_ioctl)(struct fb_info *info, unsigned int cmd, unsigned long arg);

        /* teardown any resources to do with this framebuffer */
        void (*fb_destroy)(struct fb_info *info);
};


#ifdef CONFIG_DEVFRAMEBUFFER
/* low-level drivers must call this register function first */
int register_framebuffer(struct fb_info *fb_info);

/* Higher level drivers may access fb screen directly */
unsigned char *framebuffer_get(void);
int framebuffer_setcmap(uint32_t *cmap);

/* kernel init */
int fb_init(void);
#else
#  define register_framebuffer(...) ((-ENOENT))
#  define fb_init() ((-ENOENT))
#endif

#endif
