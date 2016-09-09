#ifndef FRAMEBUFFER_INCLUDED
#define FRAMEBUFFER_INCLUDED

#include <stdint.h>
#include "frosted.h"

struct fb_ops;

enum fb_pixel_format {
    FB_PF_ARGB8888,
    FB_PF_RGB888,
    FB_PF_RGB565,
    FB_PF_ARGB15555,
    FB_PF_ARGB4444,
    FB_PF_CMAP256
};

struct fb_var_screeninfo {
    uint32_t xres; /* visible resolution */ 
    uint32_t yres; 
    uint32_t xoffset; /* offset from virtual to visible */ 
    uint32_t yoffset; /* resolution */ 

    uint32_t bits_per_pixel;
    enum fb_pixel_format pixel_format;

    /* Timing: All values in pixclocks, except pixclock (of course) */ 
    // __u32 pixclock; /* pixel clock in ps (pico seconds) */ 
    // __u32 left_margin; /* time from sync to picture */ 
    // __u32 right_margin; /* time from picture to sync */ 
    // __u32 upper_margin; /* time from sync to picture */ 
    // __u32 lower_margin; 
    // __u32 hsync_len; /* length of horizontal sync */ 
    // __u32 vsync_len; /* length of vertical sync */ 
    // __u32 sync; /* see FB_SYNC_* */ 
    // __u32 vmode; /* see FB_VMODE_* */ 
};

/* Device independent colormap information. You can get and set the colormap using the FBIOGETCMAP and FBIOPUTCMAP ioctls.  */
struct fb_cmap {
    /* For now, entries have to by 32 bits, with AARRGGBB format */
    uint32_t *start;         /* First entry  */
    uint32_t len;           /* Number of entries */
    //uint16_t *red;          /* Red values   */
    //uint16_t *green;
    //uint16_t *blue;
    //uint16_t *transp;       /* transparency, can be NULL */
};

struct fb_info {
        struct fb_var_screeninfo var;   /* Current var */
        //struct fb_fix_screeninfo fix;   /* Current fix */
        struct fb_cmap cmap;            /* Current cmap */
        //struct list_head modelist;      /* mode list */
        struct fb_videomode *mode;      /* current mode */
        //struct backlight_device *bl_dev;

        struct fb_ops *fbops;
        struct device *dev;             /* This is this fb device */
        volatile uint8_t *screen_buffer;   /* Framebuffer address */
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
        int (*fb_setcmap)(struct fb_cmap *cmap, struct fb_info *info);

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

/* kernel init */
int fb_init(void);
#else
#  define register_framebuffer(...) ((-ENOENT))
#  define fb_init() ((-ENOENT))
#endif

#endif
