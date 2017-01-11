/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "malloc.h"
#include "framebuffer.h"
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/ltdc.h>
#include <unicore-mx/stm32/spi.h>
#include "gpio.h"
#include "spi.h"

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
#define FB_BPP      (16) /* hardcoded CLUT256 - 8 bit per pixel */

#define SET_WRITE_SINGLE_CMD(x)         (x & (~(0xC0)))


/* Private function prototypes -----------------------------------------------*/
static void ltdc_config(void); 
static void ltdc_pinmux(void); 
static void ltdc_clock(void); 
static int ltdc_config_layer(struct fb_info *fb);

struct dev_ili9341 {
    struct spi_slave sl; /* First argument, for inheritance */
    struct device * dev;
    const struct gpio_config *pio_cs;
};

struct ili0341_ctrl_reg
{
    uint8_t reg;
    uint8_t data;
};

static struct dev_ili9341 ILI9341;


static struct module mod_ltdc = {
    .family = FAMILY_FILE,
    .name = "lcd-controller",
};

/* Function to write a register to ILI9341 through SPI  */
static void ili9341_write(uint8_t data)
{
    /* set CS low */
    gpio_clear(ILI9341.pio_cs->base, ILI9341.pio_cs->pin);
    //spi_xfer(SPI5, SET_WRITE_SINGLE_CMD(reg));
    spi_xfer(SPI5, data);

    /* set CS high */
    gpio_set(ILI9341.pio_cs->base, ILI9341.pio_cs->pin);
}

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
  //fb->var.pixel_format = FB_PF_RGB565;
  fb->var.pixel_format = FB_PF_ARGB8888;
  fb->var.smem_len = fb->var.xres * fb->var.yres * (fb->var.bits_per_pixel/8);
  fb->var.type = FB_TYPE_PIXELMAP;

  /* Allocate framebuffer memory */
  fb->screen_buffer = f_malloc(MEM_USER, fb->smem_len);
  if (!fb->screen_buffer)
  {
      return -1;
  }
  fb->smem_start = fb->screen_buffer;

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
  ltdc_reload(LTDC_SRCR_VBR);

  ltdc_ctrl_enable(LTDC_GCR_LTDC_ENABLE);

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
  ltdc_ctrl_enable(LTDC_GCR_PCPOL_ACTIVE_HIGH); /* Active Low Pixel Clock */

  ltdc_set_background_color(0, 0, 0);
//    LTDC_IER = LTDC_IER_RRIE;
//    nvic_enable_irq(NVIC_LCD_TFT_IRQ);

  //ltdc_ctrl_enable(LTDC_GCR_LTDC_ENABLE);
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
    //gpio_set(GPIOD, GPIO13);
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
    //ltdc_config_clut(cmap, 256);
    //ltdc_enable_clut();
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
            .speed=GPIO_OSPEED_50MHZ, 
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

    /* PF10 */
    g.base = GPIOF;
    g.pin = GPIO10;
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


    /* LCD_DISP control PC2 (enable) */
    g.base = GPIOC;
    g.pin = GPIO2;
    gpio_create(&mod_ltdc, &g);

    /* LCD_BL control PD13 (output) */
    g.base = GPIOD;
    g.pin = GPIO13;
    gpio_create(&mod_ltdc, &g);

    ltdc_screen_on();
}

/*
 * void lcd_command(cmd, delay, args, arg_ptr)
 *
 * All singing all dancing 'do a command' feature. Basically it
 * sends a command, and if args are present it sets 'data' and
 * sends those along too.
 */
static void
lcd_command(uint8_t cmd, int delay, int n_args, const uint8_t *args)
{
    //uint32_t timeout;
    int i, now;

    //gpio_clear(ILI9341.pio_cs->base, ILI9341.pio_cs->pin);
    //gpio_clear(GPIOC, GPIO2); /* Select the LCD */
    //rx_pend++;
    ili9341_write(cmd);
    /* We need to wait until it is sent, if we turn on the Data
     * line too soon, it ends up confusing the display to thinking
     * its a data transfer, as it samples the D/CX line on the last
     * bit sent.
     */
        now = jiffies;
        //while (now + 10 > jiffies) {};

    //for (timeout = 0; (timeout < 1000) && (rx_pend); timeout++) {
    //    continue;
    //}
    //rx_pend = 0;        /* sometimes, at 10Mhz we miss this */
    if (n_args) {
        gpio_set(GPIOD, GPIO13); /* Set the D/CX pin */
        for (i = 0; i < n_args; i++) {
            //rx_pend++;
            ili9341_write(*(args+i));
        }
        /* This wait so that we don't pull CS too soon after
         * sending the last byte of data.
         */
        //for (timeout = 0; (timeout < 1000) && (rx_pend); timeout++) {
        //    continue;
        //}
        now = jiffies;
        //while (now + 10 > jiffies) {};

    }
    //gpio_set(ILI9341.pio_cs->base, ILI9341.pio_cs->pin);
    //gpio_set(GPIOC, GPIO2);    /* Turn off chip select */
    gpio_clear(GPIOD, GPIO13); /* always reset D/CX */
    if (delay) {
        now = jiffies;
        //while (now + 50 > jiffies) {};

        //msleep(delay); /* wait, if called for */
    }
}

/* Notes on the less obvious ILI9341 commands: */

/*
 * ILI9341 datasheet, pp 46-49:
 *
 *     RCM[1:0} = 0b10    command 0xb0
 *     DPI[2:0] = 0b110   command 0x3a
 *     RIM      = 0       command 0xf6
 *     PCDIV    = ????    command 0xB6
 *
 * Pp 239-240:
 *     external fosc = DOTCLK / (2 * (PCDIV + 1))
 *
 * ("Cube" is how the STM32F4Cube demo software sets the register.
 *  "Chuck" is ChuckM's lcd-serial demo, first revision.)
 *
 * Command 0x3A: COLMOD: Pixel Format Set  LCD_PIXEL_FORMAT
 *                Reset              Cube   Chuck
 *      DPI[2:0]   110 (18 bit/pix)   110    101 (16 bit/pix)
 *      DBI[2:0]   110 (18 bit/pix)   110    101 (16 bit/pix)
 *
 * Command 0xB0: RGB Interface Signal      LCD_RGB_INTERFACE
 *                Reset              Cube
 *      Bypass:      0 (direct)        1 (memory)
 *      RCM[1:0]    10                10
 *      VSPL         0 (low)           0
 *      HSPL         0 (low)           0
 *      DPL          0 (rising)        1 (falling)
 *      EPL          1 (low)           0 (high)
 *
 * Command 0xB6: Display Function Control  LCD_DFC
 *                Reset              Cube 0A A7 27 04
 *      PTG[1:0]    10                10
 *      PT[1:0]     10                10
 *      REV          1                 1
 *      GS           0                 0
 *      SS           0 (S1->S720)      1 (S720->S1)
 *      SM           0                 0
 *      ISC[3:0]  0010 (5 frames)   0111 (15 frames)
 *      NL[5:0]   100111          100111
 *      PCDIV[5:0]   ?            000100
 *   S720->S1 moves the origin from the lower left corner to lower right
 *   (viewing the board so the silkscreen is upright)
 *
 * Command 0xF6: Interface Control         LCD_INTERFACE
 *               Reset              Cube  01 00 06
 *      MY_EOR       0                 0
 *      MX_EOR       0                 0
 *      MV_EOR       0                 0
 *      BGR_EOR      0                 0
 *      WEMODE       1 (wrap)          1
 *      EPF[1:0]    00                00
 *      MDT[1:0]    00                00
 *      ENDIAN       0 (MSB first)     0
 *      DM[1:0]     00 (int clk)      01 (RGB ifc)
 *      RM           0 (sys ifc)       1 (RGB ifc)
 *      RIM          0 (1 xfr/pix)     0
 */

/* ILI9341 command definitions */

/* Regulative[sic] Command Set */
#define ILI_NOP                 0x00
#define ILI_RESET               0x01
#define ILI_RD_DID              0x04
#define ILI_RD_STS              0x09
#define ILI_RD_PWR_MODE         0x0a
#define ILI_RD_MADCTL           0x0b
#define ILI_RD_PXL_FMT          0x0c
#define ILI_PD_IMG_FMT          0x0d
#define ILI_RD_SIG_MODE         0x0e
#define ILI_RD_DIAG_RSLT        0x0f
#define ILI_ENTER_SLEEP         0x10
#define ILI_SLEEP_OUT           0x11
#define ILI_PARTIAL_ON          0x12
#define ILI_NORMAL_MODE_ON      0x13
#define ILI_INVERSE_ON          0x20
#define ILI_INVERSE_OFF         0x21
#define ILI_GAMMA_SET           0x26
#define ILI_DISP_OFF            0x28
#define ILI_DISP_ON             0x29
#define ILI_CAS                 0x2a
#define ILI_PAS                 0x2b
#define ILI_MEM_WRITE           0x2c
#define ILI_COLOR_SET           0x2d
#define ILI_MEM_READ            0x2e
#define ILI_PARTIAL_AREA        0x30
#define ILI_VERT_SCROLL_DEF     0x33
#define ILI_TEAR_EFF_OFF        0x34
#define ILI_TEAR_EFF_ON         0x35
#define ILI_MEM_ACC_CTL         0x36
#define ILI_V_SCROLL_START      0x37
#define ILI_IDLE_OFF            0x38
#define ILI_IDLE_ON             0x39
#define ILI_PIX_FMT_SET         0x3a
#define ILI_WR_MEM_CONT         0x3c
#define ILI_RD_MEM_CONT         0x3e
#define ILI_SET_TEAR_LINE       0x44
#define ILI_GET_SCANLINE        0x45
#define ILI_WR_BRIGHTNESS       0x51
#define ILI_RD_BRIGHTNESS       0x52
#define ILI_WR_CTRL             0x53
#define ILI_RD_CTRL             0x54
#define ILI_WR_CABC             0x55
#define ILI_RD_CABC             0x56
#define ILI_WR_CABC_MIN         0x5e
#define ILI_RD_CABC_MAX         0x5f
#define ILI_RD_ID1              0xda
#define ILI_RD_ID2              0xdb
#define ILI_RD_ID3              0xdc

/* Extended Command Set */
#define ILI_RGB_IFC_CTL         0xb0
#define ILI_FRM_CTL_NORM        0xb1
#define ILI_FRM_CTL_IDLE        0xb2
#define ILI_FRM_CTL_PART        0xb3
#define ILI_INVERSE_CTL         0xb4
#define ILI_PORCH_CTL           0xb5
#define ILI_FUNC_CTL            0xb6
#define ILI_ENTRY_MODE_SET      0xb7
#define ILI_BL_CTL_1            0xb8
#define ILI_BL_CTL_2            0xb9
#define ILI_BL_CTL_3            0xba
#define ILI_BL_CTL_4            0xbb
#define ILI_BL_CTL_5            0xbc
#define ILI_BL_CTL_7            0xbe
#define ILI_BL_CTL_8            0xbf
#define ILI_PWR_CTL_1           0xc0
#define ILI_PWR_CTL_2           0xc1
#define ILI_VCOM_CTL_1          0xc5
#define ILI_VCOM_CTL_2          0xc7
#define ILI_NV_MEM_WR           0xd0
#define ILI_NV_MEM_PROT_KEY     0xd1
#define ILI_NV_MEM_STATUS_RD    0xd2
#define ILI_RD_ID4              0xd3
#define ILI_POS_GAMMA           0xe0
#define ILI_NEG_GAMMA           0xe1
#define ILI_GAMMA_CTL_1         0xe2
#define ILI_GAMMA_CTL_2         0xe3
#define ILI_IFC_CTL             0xf6

/*
 * This structure defines the sequence of commands to send
 * to the Display in order to initialize it. The AdaFruit
 * folks do something similar, it helps when debugging the
 * initialization sequence for the display.
 */

#define MAX_INLINE_ARGS (sizeof(uint8_t *))
struct tft_command {
  uint16_t delay;   /* If you need a delay after */
  uint8_t cmd;    /* command to send */
  uint8_t n_args;   /* How many arguments it has */
  union {
    uint8_t args[MAX_INLINE_ARGS]; /* The first four arguments */
    const uint8_t *aptr; /* More than four arguemnts */
  };
};

static const uint8_t pos_gamma_args[] = { 0x0F, 0x29, 0x24, 0x0C, 0x0E,
            0x09, 0x4E, 0x78, 0x3C, 0x09,
            0x13, 0x05, 0x17, 0x11, 0x00 };
static const uint8_t neg_gamma_args[] = { 0x00, 0x16, 0x1B, 0x04, 0x11,
            0x07, 0x31, 0x33, 0x42, 0x05,
            0x0C, 0x0A, 0x28, 0x2F, 0x0F };

/*
 * These are the commands we're going to send to the
 * display to initialize it. We send them all, in sequence
 * with occasional delays. Commands that require data bytes
 * as arguments, indicate how many bytes to pull out the
 * above array to include.
 *
 * The sequence was pieced together from the ST Micro demo
 * code, the data sheet, and other sources on the web.
 */
const struct tft_command initialization[] = {
  {  0, ILI_PWR_CTL_1,        1, .args = { 0x10 } },
  {  0, ILI_PWR_CTL_2,        1, .args = { 0x10 } },
  {  0, ILI_VCOM_CTL_1,       2, .args = { 0x45, 0x15 } },
  {  0, ILI_VCOM_CTL_2,       1, .args = { 0x90 } },
  {  0, ILI_MEM_ACC_CTL,      1, .args = { 0x08 } },
  {  0, ILI_RGB_IFC_CTL,      1, .args = { 0xc0 } },
  {  0, ILI_IFC_CTL,          3, .args = { 0x01, 0x00, 0x06 } },
  {  0, ILI_GAMMA_SET,        1, .args = { 0x01 } },
  {  0, ILI_POS_GAMMA,       15, .aptr = pos_gamma_args },
  {  0, ILI_NEG_GAMMA,       15, .aptr = neg_gamma_args },
  { +5, ILI_SLEEP_OUT,        0, .args = {} },
  {  0, ILI_DISP_ON,          0, .args = {} },
};

static void
initialize_display(const struct tft_command cmds[], size_t cmd_count)
{
  size_t i;

  for (i = 0; i < cmd_count; i++) {
    uint8_t arg_count = cmds[i].n_args;
    const uint8_t *args = cmds[i].args;
    if (arg_count > MAX_INLINE_ARGS) {
      args = cmds[i].aptr;
    }
    lcd_command(cmds[i].cmd, cmds[i].delay, arg_count, args);
  }
}

struct gpio_config ili9341_pio = {
    .base=GPIOC,
    .pin=GPIO2,
    .mode=GPIO_MODE_OUTPUT,
    .pullupdown=GPIO_PUPD_PULLUP
};


static void ili9341_init(void)
{
    memset(&ILI9341, 0, sizeof(struct dev_ili9341));

   /* Populate spi_slave struct */
    ILI9341.sl.bus = 5;
    //ILI9341.sl.isr = ili9341_isr;
    ILI9341.pio_cs = &ili9341_pio;
    gpio_create(&mod_ltdc, &ili9341_pio);
    gpio_set(ILI9341.pio_cs->base, ILI9341.pio_cs->pin);
 
    /* Set up the display */
    initialize_display(initialization,
         sizeof(initialization) / sizeof(initialization[0]));
}


/* DRIVER INIT */
void ltdc_init(void)
{
    lcd_pinmux();
    ltdc_clock(); 
    ltdc_config(); /* Configure LCD : Only one layer is used */
    ltdc_config_layer(&ltdc_info);
    ili9341_init();
    register_framebuffer(&ltdc_info);
    register_module(&mod_ltdc);
}
