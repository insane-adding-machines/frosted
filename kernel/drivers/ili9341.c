/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "malloc.h"
#include "framebuffer.h"
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/ltdc.h>
#include "gpio.h"

/* ILI9341 LCD screen ... */
#define  ILI9341_WIDTH               ((uint16_t)240)  /* LCD PIXEL WIDTH            */
#define  ILI9341_HEIGHT              ((uint16_t)320)  /* LCD PIXEL HEIGHT           */
#define  ILI9341_HSYNC               ((uint16_t)10)   /* Horizontal synchronization */
#define  ILI9341_HBP                 ((uint16_t)20)   /* Horizontal back porch      */
#define  ILI9341_HFP                 ((uint16_t)10)   /* Horizontal front porch     */
#define  ILI9341_VSYNC               ((uint16_t)2)   /* Vertical synchronization   */
#define  ILI9341_VBP                 ((uint16_t)2)    /* Vertical back porch        */
#define  ILI9341_VFP                 ((uint16_t)4)    /* Vertical front porch       */
//#define  ILI9341_FREQUENCY_DIVIDER    5               /* LCD Frequency divider      */
//#define REFRESH_RATE 70 /* Hz */

#define FB_WIDTH    ILI9341_WIDTH
#define FB_HEIGTH   ILI9341_HEIGHT
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

  /* TODO: move to check / set var */
  fb->var.xres = FB_WIDTH;
  fb->var.yres = FB_HEIGTH;
  fb->var.bits_per_pixel = FB_BPP;
  fb->var.pixel_format = FB_PF_RGB565;
  //fb->var.pixel_format = FB_PF_ARGB8888;

  /* Allocate framebuffer memory */
  fb->screen_buffer = f_malloc(MEM_USER, fb->var.xres * fb->var.yres * (fb->var.bits_per_pixel/8));
  if (!fb->screen_buffer)
  {
      return -1;
  }

  /* Windowing configuration */ 
  ltdc_setup_windowing(LTDC_LAYER_2, fb->var.xres, fb->var.xres);

  /* Specifies the pixel format */
  switch (fb->var.pixel_format)
  {
      case FB_PF_RGB565:
          format = LTDC_LxPFCR_RGB565;
          break;
      case FB_PF_ARGB8888:
          format = LTDC_LxPFCR_ARGB8888;
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
  ltdc_set_tft_sync_timings(ILI9341_HSYNC, ILI9341_VSYNC,
			                ILI9341_HBP,   ILI9341_VBP,
			                ILI9341_WIDTH, ILI9341_HEIGHT,
			                ILI9341_HFP,   ILI9341_VFP);

  ltdc_set_background_color(0, 0, 0);
  ltdc_ctrl_enable(LTDC_GCR_LTDC_ENABLE);
}


static void ltdc_clock(void)
{

  /*
   * The datasheet says (Figure 16, page 151):
   *     The LCD-TFT clock comes from PLLSAI.
   *     PLLSRC selects either HSI or HSE.
   *     PLLSAI's input clock is either HSI or HSE divided by PLLM.
   *     PLLSAI's PLLLCDCLK output is the input * PLLSAIN / PLLSAIR.
   *     LCD-TFT clock is PLLLCDCLK divided by PLLSAIDIVR.
   *
   * PLLSRC and PLLM are in the RCC_PLLCFGR register.
   * PLLSAIN and PLLSAIR are in RCC_PLLSAICFGR.
   * PLLSAIDIVR is in RCC_DCKCFGR;
   *
   * In our case,
   * PLLSRC already selected HSE, which is 8 MHz.
   * PLLM is already set to 8.  8 MHz / 8 = 1 MHz.
   * We set PLLSAIN = 192 and PLLSAIR = 4.  1 MHz * 192 / 4 = 48 MHz.
   * We set PLLSAIDIVR to 8.  48 MHz / 8 = 6 MHz.
   * So the LCD-TFT pixel clock is 6 MHz.
   *
   * The number of clocks per frame is
   * (VSYNC + VBP + LCD_HEIGHT + VFP) * (HSYNC + HBP + LCD_WIDTH + HFP) =
   * (2 + 2 + 320 + 4) * (10 + 20 + 240 + 10) = 91840.
   *
   * So the refresh frequency is 6 MHz / 91840 ~= 65.6 Hz.
   */
  /* Disable PLLSAI */
  RCC_CR &= ~RCC_CR_PLLSAION;
  while((RCC_CR & (RCC_CR_PLLSAIRDY))) {};

  uint32_t sain = 192;
  uint32_t saiq = (RCC_PLLSAICFGR >> RCC_PLLSAICFGR_PLLSAIQ_SHIFT) &
      RCC_PLLSAICFGR_PLLSAIQ_MASK;
  uint32_t sair = 4;
  RCC_PLLSAICFGR = (sain << RCC_PLLSAICFGR_PLLSAIN_SHIFT |
        saiq << RCC_PLLSAICFGR_PLLSAIQ_SHIFT |
        sair << RCC_PLLSAICFGR_PLLSAIR_SHIFT);
  RCC_DCKCFGR |= RCC_DCKCFGR_PLLSAIDIVR_DIVR_8;
  RCC_CR |= RCC_CR_PLLSAION;
  while(!(RCC_CR & (RCC_CR_PLLSAIRDY))) {};
  RCC_APB2ENR |= RCC_APB2ENR_LTDCEN;
}

static int ltdc_blank(struct fb_info *fb)
{
    uint32_t pixels = (fb->var.xres * fb->var.yres * (fb->var.bits_per_pixel/8));
    memset((void *)fb->screen_buffer, 0x0, pixels);
}

static void ltdc_screen_on(void)
{
    /* Assert display enable LCD_DISP pin */
    gpio_set(GPIOF, GPIO10);
    /* Assert backlight LCD_BL_CTRL pin */
    //gpio_set(GPIOK, GPIO3);
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




    /* PA3 */
    g.base = GPIOA;
    g.pin = GPIO3;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PA6 */
    g.base = GPIOA;
    g.pin = GPIO6;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PA11 */
    g.base = GPIOA;
    g.pin = GPIO11;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PA12 */
    g.base = GPIOA;
    g.pin = GPIO12;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PB0 */
    g.base = GPIOB;
    g.pin = GPIO0;
    g.af = GPIO_AF9;
    gpio_create(&mod_ltdc, &g);

    /* PB1 */
    g.base = GPIOB;
    g.pin = GPIO1;
    g.af = GPIO_AF9;
    gpio_create(&mod_ltdc, &g);

    /* PC7 */
    g.base = GPIOC;
    g.pin = GPIO7;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PC10 */
    g.base = GPIOC;
    g.pin = GPIO10;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PD3 */
    g.base = GPIOD;
    g.pin = GPIO3;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PD6 */
    g.base = GPIOD;
    g.pin = GPIO6;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PG6 */
    g.base = GPIOG;
    g.pin = GPIO6;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PG10 */
    g.base = GPIOG;
    g.pin = GPIO10;
    g.af = GPIO_AF9;
    gpio_create(&mod_ltdc, &g);

    /* PG11 */
    g.base = GPIOG;
    g.pin = GPIO11;
    g.af = GPIO_AF11;
    gpio_create(&mod_ltdc, &g);

    /* PG12 */
    g.base = GPIOG;
    g.pin = GPIO12;
    g.af = GPIO_AF9;
    gpio_create(&mod_ltdc, &g);


    /* PB8:PB11*/
    for (i = 8; i < 12; i++) {
        g.base = GPIOB;
        g.pin = (1 << i);
        g.af = GPIO_AF14;
        gpio_create(&mod_ltdc, &g);
    }

    /* PG7 */
    g.base = GPIOG;
    g.pin = GPIO7;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PC6 */
    g.base = GPIOC;
    g.pin = GPIO6;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PA4 */
    g.base = GPIOA;
    g.pin = GPIO4;
    g.af = GPIO_AF14;
    gpio_create(&mod_ltdc, &g);

    /* PC2 */
    g.base = GPIOC;
    g.pin = GPIO2;
    g.mode = GPIO_MODE_OUTPUT;
    gpio_create(&mod_ltdc, &g);

    /* PD12 */
    g.base = GPIOD;
    g.pin = GPIO12;
    gpio_create(&mod_ltdc, &g);

    /* PD11 */
    g.base = GPIOD;
    g.pin = GPIO11;
    gpio_create(&mod_ltdc, &g);

    /* PD13 */
    g.base = GPIOD;
    g.pin = GPIO13;
    gpio_create(&mod_ltdc, &g);

    /* PF7 */
    g.base = GPIOF;
    g.pin = GPIO7;
    gpio_create(&mod_ltdc, &g);

    /* PF9 */
    g.base = GPIOF;
    g.pin = GPIO9;
    gpio_create(&mod_ltdc, &g);


    /* LCD_DISP control PF10 (enable) */
    g.base = GPIOF;
    g.pin = GPIO10;
    g.mode = GPIO_MODE_AF;
    g.af = GPIO_AF14;
    g.name = "display";
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

