#include "frosted.h"
/* this define lets the init code know that you are using a GPIO as a card
 * detect pin */
#define SDIO_HAS_CARD_DETECT

enum SDIO_CLOCK_DIV {
    SDIO_24MHZ = 0,
    SDIO_16MHZ,
    SDIO_12MHZ,
    SDIO_8MHZ,
    SDIO_4MHZ,
    SDIO_1MHZ,
    SDIO_400KHZ
};

enum SDIO_POWER_STATE {
    SDIO_POWER_ON,
    SDIO_POWER_OFF
};

#define SDIO_CARD_CCS(c)     (((c)->ocr & 0x40000000) != 0)
#define SDIO_CARD_UHS2(c)   (((c)->ocr & 0x40000000) != 0)
#define SDIO_CARD_LVOK(c)   (((c)->ocr & 0x01000000) != 0)

typedef struct SDIO_CARD_DATA {
    uint32_t    props;
    uint32_t    ocr;
    uint32_t    cid[4];
    uint32_t    csd[4];
    uint32_t    scr[2];
    uint32_t    status[16];
    uint32_t    size;
    uint16_t    rca;
} * SDIO_CARD;

struct dev_sd {
    struct device *dev;
    uint32_t base;
    uint32_t *rcc_rst_reg;
    uint32_t rcc_rst;
    SDIO_CARD card;
};

int stm32_sdio_bus(struct dev_sd *sd, int bits, enum SDIO_CLOCK_DIV freq);
int stm32_sdio_init(void);
void stm32_sdio_reset(struct dev_sd *sd, enum SDIO_POWER_STATE state);
SDIO_CARD stm32_sdio_open(struct dev_sd *sd);
int stm32_sdio_command(struct dev_sd *sd, uint32_t cmd, uint32_t arg);
int stm32_sdio_readblock(SDIO_CARD, uint32_t lba, uint8_t *buf);
int stm32_sdio_writeblock(SDIO_CARD, uint32_t lba, uint8_t *buf);
//int stm32_sdio_status(SDIO_CARD);
void stm32_sdio_print_log(int console, int number_of_entries);
const char *stm32_sdio_errmsg(int err);
uint32_t stm32_sdio_bit_slice(uint32_t a[], int bits, int msb, int lsb);

/* API for sd_bus clock setting */
enum SD_CLOCK_DIV {
    CLOCK_24MHZ = 0,
    CLOCK_16MHZ,
    CLOCK_12MHZ,
    CLOCK_8MHZ,
    CLOCK_4MHZ,
    CLOCK_1MHZ,
    CLOCK_400KHZ
};

//int stm32_sd_bus(int bits, enum SD_CLOCK_DIV freq);
