#ifndef INCLUDE_FONT_DEFINITION
#define INCLUDE_FONT_DEFINITION

#ifdef CONFIG_FONT_7x6
#   define FONT_HEIGHT 7
#   define FONT_WIDTH  6
#endif

#ifdef CONFIG_FONT_8x8
#   define FONT_HEIGHT 8
#   define FONT_WIDTH  8
#endif

extern const unsigned char fb_font[256][FONT_HEIGHT];

#endif
