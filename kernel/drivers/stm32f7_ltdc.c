/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "malloc.h"
#include "framebuffer.h"
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/ltdc.h>
#include "gpio.h"

/* STM32F7-Discovery screen ... */
#define  RK043FN48H_WIDTH               ((uint16_t)480)  /* LCD PIXEL WIDTH            */
#define  RK043FN48H_HEIGHT              ((uint16_t)272)  /* LCD PIXEL HEIGHT           */
#define  RK043FN48H_HSYNC               ((uint16_t)41)   /* Horizontal synchronization */
#define  RK043FN48H_HBP                 ((uint16_t)13)   /* Horizontal back porch      */
#define  RK043FN48H_HFP                 ((uint16_t)32)   /* Horizontal front porch     */
#define  RK043FN48H_VSYNC               ((uint16_t)10)   /* Vertical synchronization   */
#define  RK043FN48H_VBP                 ((uint16_t)2)    /* Vertical back porch        */
#define  RK043FN48H_VFP                 ((uint16_t)2)    /* Vertical front porch       */
#define  RK043FN48H_FREQUENCY_DIVIDER    5               /* LCD Frequency divider      */

#define FB_WIDTH    RK043FN48H_WIDTH
#define FB_HEIGTH   RK043FN48H_HEIGHT
//#define FB_BPP      (16) /* hardcoded RGB565 - 16 bits per pixel */
#define FB_BPP      (8) /* hardcoded CLUT256 - 8 bit per pixel */

/* Private function prototypes -----------------------------------------------*/
static void ltdc_config(void); 
static void ltdc_pinmux(void); 
static void ltdc_clock(void); 
static int ltdc_config_layer(struct fb_info *fb);

static struct module mod_ltdc = {
    .family = FAMILY_FILE,
    .name = "lcd-controller",
};

/* Private functions ---------------------------------------------------------*/

static void ltdc_destroy(struct fb_info *fb)
{
    // TODO: disable framebuffer
    
    if (fb && fb->screen_buffer)
        f_free((void *)fb->screen_buffer);
}

static int ltdc_config_layer(struct fb_info *fb)
{
  uint32_t format = 0; 

  fb->var.xres = FB_WIDTH;
  fb->var.yres = FB_HEIGTH;
  fb->var.bits_per_pixel = FB_BPP;
  //fb->var.pixel_format = FB_PF_RGB565;
  fb->var.pixel_format = FB_PF_CMAP256;
  fb->var.smem_len = fb->var.xres * fb->var.yres * (fb->var.bits_per_pixel/8);
  fb->var.type = FB_TYPE_PIXELMAP;

  /* Allocate framebuffer memory */
  fb->screen_buffer = u_malloc(fb->var.smem_len);
  if (!fb->screen_buffer)
  {
      return -1;
  }
  fb->var.smem_start = fb->screen_buffer;

  /* Windowing configuration */ 
  ltdc_setup_windowing(LTDC_LAYER_2, fb->var.xres, fb->var.xres);

  /* Specifies the pixel format */
  switch (fb->var.pixel_format)
  {
      case FB_PF_RGB565:
          format = LTDC_LxPFCR_RGB565;
          break;
      case FB_PF_CMAP256:
          format = LTDC_LxPFCR_L8;
          break;
      default:
          format = LTDC_LxPFCR_RGB565;
          break;
  }

  ltdc_set_pixel_format(LTDC_LAYER_2, format);

  /* Default color values */
  ltdc_set_default_colors(LTDC_LAYER_2, 0, 0, 0, 0);

  /* Constant alpha */
  ltdc_set_constant_alpha(LTDC_LAYER_2, 255);

  /* Blending factors */
  ltdc_set_blending_factors(LTDC_LAYER_2, LTDC_LxBFCR_BF1_CONST_ALPHA, LTDC_LxBFCR_BF2_CONST_ALPHA);

  /* Framebuffer memory address */
  ltdc_set_fbuffer_address(LTDC_LAYER_2, (uint32_t)fb->screen_buffer);

  /* Configures the color frame buffer pitch in byte */
  ltdc_set_fb_line_length(LTDC_LAYER_2, fb->var.xres * (fb->var.bits_per_pixel/8), fb->var.xres * (fb->var.bits_per_pixel/8));

  /* Configures the frame buffer line number */
  ltdc_set_fb_line_count(LTDC_LAYER_2, fb->var.yres);

  /* Enable layer 1 */
  ltdc_layer_ctrl_enable(LTDC_LAYER_2, LTDC_LxCR_LAYER_ENABLE);

  /* Sets the Reload type */
  ltdc_reload(LTDC_SRCR_IMR);

  return 0;
}

/**
  * @brief LCD Configuration.
  * @note  This function Configure tha LTDC peripheral :
  *        1) Configure the Pixel Clock for the LCD
  *        2) Configure the LTDC Timing and Polarity
  *        3) Configure the LTDC Layer 1 :
  *           - The frame buffer is located at FLASH memory
  *           - The Layer size configuration : 480x272                      
  * @retval
  *  None
  */
static void ltdc_config(void)
{ 
  /* LTDC Initialization */
  ltdc_ctrl_disable(LTDC_GCR_HSPOL_ACTIVE_HIGH); /* Active Low Horizontal Sync */
  ltdc_ctrl_disable(LTDC_GCR_VSPOL_ACTIVE_HIGH); /* Active Low Vertical Sync */
  ltdc_ctrl_disable(LTDC_GCR_DEPOL_ACTIVE_HIGH); /* Active Low Date Enable */
  ltdc_ctrl_disable(LTDC_GCR_PCPOL_ACTIVE_HIGH); /* Active Low Pixel Clock */
  
  /* Configure the LTDC */  
  ltdc_set_tft_sync_timings(RK043FN48H_HSYNC, RK043FN48H_VSYNC,
			                RK043FN48H_HBP,   RK043FN48H_VBP,
			                RK043FN48H_WIDTH, RK043FN48H_HEIGHT,
			                RK043FN48H_HFP,   RK043FN48H_VFP);

  ltdc_set_background_color(0, 0, 0);
  ltdc_ctrl_enable(LTDC_GCR_LTDC_ENABLE);
}


static void ltdc_clock(void)
{
  /* LCD clock configuration */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/5 = 38.4 Mhz */
  /* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_4 = 38.4/4 = 9.6Mhz */

  /* Disable PLLSAI */
  RCC_CR &= ~RCC_CR_PLLSAION;
  while((RCC_CR & (RCC_CR_PLLSAIRDY))) {};

  /* N and R are needed,
   * P and Q are not needed for LTDC */
  RCC_PLLSAICFGR &= ~RCC_PLLSAICFGR_PLLSAIN_MASK;
  RCC_PLLSAICFGR |= 192 << RCC_PLLSAICFGR_PLLSAIN_SHIFT;
  RCC_PLLSAICFGR &= ~RCC_PLLSAICFGR_PLLSAIR_MASK;
  RCC_PLLSAICFGR |= 5 << RCC_PLLSAICFGR_PLLSAIR_SHIFT;
  RCC_DCKCFGR1 &= ~RCC_DCKCFGR1_PLLSAIDIVR_MASK;
  RCC_DCKCFGR1 |= RCC_DCKCFGR1_PLLSAIDIVR_DIVR_4;

  /* Enable PLLSAI */
  RCC_CR |= RCC_CR_PLLSAION;
  while(!(RCC_CR & (RCC_CR_PLLSAIRDY))) {};
}

static int ltdc_blank(struct fb_info *fb)
{
    uint32_t pixels = (fb->var.xres * fb->var.yres * (fb->var.bits_per_pixel/8));
    memset((void *)fb->screen_buffer, 0x0, pixels);
}

static void ltdc_screen_on(void)
{
    /* Assert display enable LCD_DISP pin */
    gpio_set(GPIOI, GPIO12);
    /* Assert backlight LCD_BL_CTRL pin */
    gpio_set(GPIOK, GPIO3);
}

void ltdc_enable_clut(void)
{
  /* Disable LTDC color lookup table by setting CLUTEN bit */
  ltdc_layer_ctrl_enable(LTDC_LAYER_2, LTDC_LxCR_CLUT_ENABLE);

  /* Sets the Reload type */
  ltdc_reload(LTDC_SRCR_IMR);
}

/* Only L8 CLUTs supported for now */
void ltdc_config_clut(uint32_t *CLUT, uint32_t size)
{
    uint32_t i = 0;

    for(i = 0; (i < size); i++)
    {
        /* Specifies the C-LUT address and RGB value */
        LTDC_LxCLUTWR(LTDC_LAYER_2) = ((i << 24) | ((uint32_t)(*CLUT) & 0xFF) | ((uint32_t)(*CLUT) & 0xFF00) | ((uint32_t)(*CLUT) & 0xFF0000));
        CLUT++;
    }
}

int ltdc_set_cmap(uint32_t *cmap, struct fb_info *info)
{
    ltdc_config_clut(cmap, 256);
    ltdc_enable_clut();
    return 0;
}


static int ltdc_open(struct fb_info *info)
{
    /* init LCD */
}

static const struct fb_ops  ltdc_fbops = {  
                                            .fb_open = ltdc_open,
                                            .fb_destroy = ltdc_destroy,
                                            .fb_blank = ltdc_blank,
                                            .fb_setcmap = ltdc_set_cmap};

static struct fb_info ltdc_info = { 
        .fbops = (struct fb_ops *)&ltdc_fbops,
};



static void lcd_pinmux(void)
{
    int i;
    struct gpio_config g = { 
            .mode=GPIO_MODE_AF,
            .speed=GPIO_OSPEED_100MHZ, 
            .optype=GPIO_OTYPE_PP, 
            .pullupdown = GPIO_PUPD_NONE
    };

    /* Enable the LTDC Clock */
    rcc_periph_clock_enable(RCC_LTDC);

    /* PE4 */
    g.base = GPIOE;
    g.pin = GPIO4;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PG12 */
    g.base = GPIOG;
    g.pin = GPIO12;
    g.af = GPIO_AF9;
    gpio_create(&mod_ltdc, &g);

    /* LTDC PI8:PI10*/
    for (i = 8; i < 11; i++) {
        g.base = GPIOI;
        g.pin = (1 << i);
        g.af = GPIO_AF14;
        gpio_create(&mod_ltdc, &g);
    }

    /* LTDC PI14:PI15 */
    for (i = 14; i < 16; i++) {
        g.base = GPIOI;
        g.pin = (1 << i);
        g.af = GPIO_AF14;
        gpio_create(&mod_ltdc, &g);
    }

    /* LTDC PJ0:PJ15 (excl. PJ12) */
    for (i = 0; i < 16; i++) {
        if (i != 12) {
            g.base = GPIOJ;
            g.pin = (1 << i);
            g.af = GPIO_AF14;
            gpio_create(&mod_ltdc, &g);
        }
    }
    
    /* LTDC PK0:PK7 (excl. PK3) */
    for (i = 0; i < 8; i++) {
        if (i != 3) {
            g.base = GPIOK;
            g.pin = (1 << i);
            g.af = GPIO_AF14;
            gpio_create(&mod_ltdc, &g);
        }
    }
    
    /* LCD_DISP control PI12 (output) */
    g.base = GPIOI;
    g.pin = GPIO12;
    g.mode = GPIO_MODE_OUTPUT;
    g.name = "display";
    gpio_create(NULL, &g);

    /* LCD_BL control PK3 (output) */
    g.base = GPIOK;
    g.pin = GPIO3;
    g.mode = GPIO_MODE_OUTPUT;
    g.name = "backlight";
    gpio_create(NULL, &g);
    ltdc_screen_on();

}


/* DRIVER INIT */
void ltdc_init(void)
{
    lcd_pinmux();
    ltdc_clock(); 
    ltdc_config(); /* Configure LCD : Only one layer is used */
    ltdc_config_layer(&ltdc_info);
    register_framebuffer(&ltdc_info);
    register_module(&mod_ltdc);
}

