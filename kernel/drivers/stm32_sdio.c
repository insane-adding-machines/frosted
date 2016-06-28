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
 *      Original Author: Chuck M. (see https://github.com/ChuckM/stm32f4-sdio-driver/)
 *      Re-adapted for frosted by: Daniele Lacamera
 *
 *      Permission to release under the terms of GPLv2 are granted by the 
 *      copyright holders.
 *
 */




/*
 * SDIO Bus Driver layer. This code sends commands and drives
 * the SDIO peripheral on the STM32F4xx, there is a layer above
 * this, the SD Card driver, which uses this driver to talk to
 * SD Cards. The SPI driver can also talk to SD Cards, hence the
 * split at this layer.
 *
 * Note that the simple implementation for the SDIO driver runs
 * in a 'polled' mode. This is easier to explain and debug and
 * sufficient for the first few projects. A more sophisticated
 * version with DMA and interrupts will follow.
 *

 *
 *
 */

#include <stdint.h>
#include <unicore-mx/stm32/sdio.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/cm3/nvic.h>
#include "stm32_sdio.h"
#include "frosted.h"
#include "device.h"

/* Frosted device driver hook */
static struct module mod_sdio;
struct dev_sd {
    struct device * dev;
    SDIO_CARD card;
};

struct dev_sd SdCard[1]; /* Multiple slots? Make this array bigger */

/*
 * sd_bus
 *
 * Set the bus width and the clock speed for the
 * SDIO bus.
 *
 * Returns 0 on success
 *      -1 illegal bit specification
 *      -2 illegal clock specification
 */
int
stm32_sd_bus(int bits, enum SD_CLOCK_DIV freq) {
    int clkreg = 0;

    switch (bits) {
        case 1:
            clkreg |= SDIO_CLKCR_WIDBUS_1;
            break;
        case 4:
            clkreg |= SDIO_CLKCR_WIDBUS_4;
            break;
        default:
            return -1;
    }
    switch (freq) {
        CLOCK_24MHZ:
            break;
        CLOCK_16MHZ:
            clkreg |= 1;
            break;
        CLOCK_12MHZ:
            clkreg |= 2;
            break;
        CLOCK_8MHZ:
            clkreg |= 8;
            break;
        CLOCK_4MHZ:
            clkreg |= 10;
            break;
        CLOCK_1MHZ:
            clkreg |= 46;
            break;
        CLOCK_400KHZ:
            clkreg |= 118;
            break;
        default:
            return -2;
    }
    clkreg |= SDIO_CLKCR_CLKEN;
    SDIO_CLKCR = clkreg;
    return 0;
}

/*
 * Some Global defines, collected here.
 */

/*
 * Not defined by default
 */
#ifndef NULL
#define NULL (void *)(0x00000000)
#endif

/*
 * Helper defines to pull out various bit fields of the CSD for
 * the size calculation.
 */
#define SDIO_CSD_VERSION(x) stm32_sdio_bit_slice(x->csd, 128, 127, 126)
#define SDIO_CSD1_CSIZE_MULT(x) stm32_sdio_bit_slice(x->csd, 128, 49, 47)
#define SDIO_CSD1_RBLKLEN(x) stm32_sdio_bit_slice(x->csd, 128, 83, 80)
#define SDIO_CSD1_CSIZE(x) stm32_sdio_bit_slice(x->csd, 128, 73, 62)
#define SDIO_CSD2_CSIZE(x) stm32_sdio_bit_slice(x->csd, 128, 69, 48)

/*
 * Conveniently swaps the bytes in a long around
 * used by the SCR code.
 */
#define byte_swap(val) \
        asm("rev %[swap], %[swap]" : [swap] "=r" (val) : "0" (val));

/*
 * sdio_bus
 *
 * Set the bus width and the clock speed for the
 * SDIO bus.
 *
 * Returns 0 on success
 *      -1 illegal bit specification
 *      -2 illegal clock specification
 */
int
stm32_sdio_bus(int bits, enum SDIO_CLOCK_DIV freq) {
    int clkreg = 0;

    switch (bits) {
        case 1:
            clkreg |= SDIO_CLKCR_WIDBUS_1;
            break;
        case 4:
            clkreg |= SDIO_CLKCR_WIDBUS_4;
            break;
        default:
            return -1;
    }
    switch (freq) {
        case SDIO_24MHZ:
            break;
        case SDIO_16MHZ:
            clkreg |= 1;
            break;
        case SDIO_12MHZ:
            clkreg |= 2;
            break;
        case SDIO_8MHZ:
            clkreg |= 8;
            break;
        case SDIO_4MHZ:
            clkreg |= 10;
            break;
        case SDIO_1MHZ:
            clkreg |= 46;
            break;
        case SDIO_400KHZ:
            clkreg |= 118;
            break;
        default:
            return -2;
    }
    clkreg |= SDIO_CLKCR_CLKEN;
    SDIO_CLKCR = clkreg;
    return 0;
}

/*
 * Reset the state of the SDIO bus and peripheral. This code tries
 * to reset the bus *AND* the card if one is plugged in. The bus
 * can be reset by software but the card is reset by powering it down.
 *
 * The SDIO_POWER_STATE tells the code which state to leave the bus in,
 * powered up or powered down.
 *
 * If the state is POWER_ON, then the bus is reset to 400Khz, 1 bit wide
 * which is what he spec requires. Once the type and capabilities of the
 * card have been determined, it can be upgraded.
 */
void
stm32_sdio_reset(enum SDIO_POWER_STATE state) {

    /* Step 1 power off the interface */
    SDIO_POWER = SDIO_POWER_PWRCTRL_PWROFF;
    /* reset the SDIO peripheral interface */
#ifdef STM32F4
    rcc_peripheral_reset(&RCC_APB2RSTR, RCC_APB2RSTR_SDIORST);
    rcc_peripheral_clear_reset(&RCC_APB2RSTR, RCC_APB2RSTR_SDIORST);
#elif defined STM32F7
    rcc_peripheral_reset(&RCC_APB2RSTR, RCC_APB2RSTR_SDMMC1RST);
    rcc_peripheral_clear_reset(&RCC_APB2RSTR, RCC_APB2RSTR_SDMMC1RST);
#endif
    if (state == SDIO_POWER_ON) {
        SDIO_POWER = SDIO_POWER_PWRCTRL_PWRON;
        stm32_sdio_bus(1, SDIO_400KHZ); // required by the spec
    }
}

/*
 * The error message catalog.
 */
static const char *__sdio_error_msgs[] = {
    "Success",
    "Command Timeout",              // -1
    "Command CRC Failure",          // -2
    "Soft Timeout (No Response)",   // -3
    "Data CRC Failure",             // -4
    "RX FIFO Overrun",              // -5
    "TX FIFO Underrun",             // -6
    "Unsupported Card"              // -7
};

#define SDIO_ESUCCESS    0
#define SDIO_ECTIMEOUT  -1
#define SDIO_ECCRCFAIL  -2
#define SDIO_ENORESP    -3
#define SDIO_EDCRCFAIL  -4
#define SDIO_ERXOVERR   -5
#define SDIO_ETXUNDER   -6
#define SDIO_EBADCARD   -7
#define SDIO_EUNKNOWN   -8

/*
 * Return a text string description of the error code.
 */
const char *
stm32_sdio_errmsg(int err) {
    return (err <= SDIO_EUNKNOWN) ? (const char *) "Unknown Error" :
                                __sdio_error_msgs[0-err];
}

/* 
 * stm32_sdio_bit_slice - helper function
 *
 * A number of the things the SDIO returns are in bit
 * fields. This code is designed to slice out a range
 * of bits and return them as a value (up to 32 bits
 * worth).
 */
uint32_t
stm32_sdio_bit_slice(uint32_t a[], int bits, int msb, int lsb) {
    uint32_t t;
    int i;

    if (((msb >= bits) || (msb < 0)) ||
        (lsb > msb) ||
        ((lsb < 0) || (lsb >= bits))) {
        kprintf("Bad Slice values.\r\n");
        return 0;
    }
    t = 0;
    for (i = msb; i > lsb; i--) {
        t |= (a[((bits-1) - i)/32] >> (i % 32)) & 0x1;
        t <<= 1;
    }
    t |= (a[((bits-1) - lsb)/32] >> (lsb % 32)) & 0x1;
    return t;
}

/*
 * A convienence define. These are the flags we care about when
 * sending a command. During command processing SDIO_STA_CMDACT
 * will be set.
 */
#define COMMAND_FLAGS   (SDIO_STA_CMDSENT |\
                         SDIO_STA_CCRCFAIL |\
                         SDIO_STA_CMDREND |\
                         SDIO_STA_CTIMEOUT)

/*
 * Send a command over the SDIO bus.
 * Passed a command (8 bit value) and an argument (32 bit value)
 * This command figures out if the command will return a short (32 bit)
 * or long (64 bit) response. It is up to the calling program to pull
 * data from the long response commands.
 * Passed:
 *          cmd - Command to execute
 *          arg - Argument to pass to the command
 *          buf - pointer to a long aligned buffer if data
 *          len - expected length of buffer (in bytes)
 */
int
stm32_sdio_command(uint32_t cmd, uint32_t arg)
{
    uint32_t    tmp_val;
    int         error = 0;

    tmp_val = SDIO_CMD & ~0x7ff;            // Read pre-existing state
    tmp_val |= (cmd & SDIO_CMD_CMDINDEX_MSK);   // Put the Command in
    tmp_val |= SDIO_CMD_CPSMEN;                 // We'll be running CPSM

    switch(cmd) {
        case 0:
            tmp_val |= SDIO_CMD_WAITRESP_NO_0;
            break;
        case 2:
        case 9:
            tmp_val |= SDIO_CMD_WAITRESP_LONG;
            break;
        default:
            tmp_val |= SDIO_CMD_WAITRESP_SHORT; // the common case
            break;
    }
    /* If a data transaction is in progress, wait for it to finish */
#if 0
    if ((cmd != 12) && (SDIO_STA & (SDIO_STA_RXACT | SDIO_STA_TXACT))) {
        // XXX: This should be an error, we don't have multithread
        tmp_val |= SDIO_CMD_WAITPEND;
    }
#endif

    /*
     * EXECUTE:
     *    o Reset all status bits
     *    o Put ARG into SDIO ARG
     *    o reset the error indicator
     *    o Enable all interrupts.
     *    o Do the command
     */
    SDIO_ICR = 0x7ff;           // Reset everything that isn't bolted down.
    SDIO_ARG = arg;
    SDIO_CMD = tmp_val;
    /*
     * In a polled mode we should be able to just read the status bits
     * directly.
     */
    tmp_val = 0;
    do {
        tmp_val |= (SDIO_STA & 0x7ff);
    } while ((SDIO_STA & SDIO_STA_CMDACT) || (! tmp_val));;
    SDIO_ICR = tmp_val;

    /*
     * Compute the error here. Which can be one of
     * -- Success (either CMDSENT or CMDREND depending on response)
     * -- Timeout (based on CTIMEOUT)
     * -- No Response (based on no response in the time alloted)
     * -- CRC Error (based on CCRCFAIL)
     */
    if (! tmp_val) {
        error = SDIO_ENORESP;
    } else if (tmp_val & SDIO_STA_CCRCFAIL) {
        error = SDIO_ECCRCFAIL;
    } else if (tmp_val & (SDIO_STA_CMDREND | SDIO_STA_CMDSENT)) {
        error = SDIO_ESUCCESS;
    } else if (tmp_val & SDIO_STA_CTIMEOUT) {
        error = SDIO_ECTIMEOUT;
    } else {
        error = SDIO_EUNKNOWN;
    }

    return error;
}


/* our static data buffer we use for data movement commands */
static uint32_t data_buf[129];
static int data_len;

/*
 * Helper function - sdio_select
 *
 * This function "selects" a card using CMD7, note that if
 * you select card 0 that deselects the card (RCA is not allowed
 * to be 0)
 */
static int
sdio_select(int rca) {
    int err;

    err = stm32_sdio_command(7, rca << 16);
    if ((rca == 0) && (err == SDIO_ECTIMEOUT)) {
        return 0;   // "cheat" a timeout selecting 0 is a successful deselect
    }
    return err;
}

/*
 * Helper function - sdio_scr
 *
 * Unlike the CID and CSD functions this function transfers data
 * so it needs to use the DPSM.
 *
 * Note that data over the wire is byte swapped so we swap it back
 * to "fix" it.
 *
 * Note when this return 0 the first two longs in the data_buf are
 * the SCR register.
 */

static int
sdio_scr(SDIO_CARD c) {
    int err;
    uint32_t    tmp_reg;
    int ndx;

    /* Select the card */
    err = sdio_select(c->rca);
    if (! err) {
        /* Set the Block Size */
        err = stm32_sdio_command(16, 8);
        if (! err) {
            /* APPCMD (our RCA) */
            err = stm32_sdio_command(55, c->rca << 16);
            if (! err) {
                SDIO_ICR = 0xFFFFFFFF; /* Clear all status flags */
                SDIO_DTIMER = 0xffffffff;
                SDIO_DLEN = 8;
                SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_3 |
                             SDIO_DCTRL_DTDIR |
                             SDIO_DCTRL_DTEN;
                /* ACMD51 - Send SCR */
                err = stm32_sdio_command(51, 0);
                if (! err) {
                    data_len = 0;
                    do {
                        tmp_reg = SDIO_STA;
                        if (tmp_reg & SDIO_STA_RXDAVL) {
                            data_buf[data_len++] = SDIO_FIFO;
                        }
                    } while (tmp_reg & SDIO_STA_RXACT);
                    if ((tmp_reg & SDIO_STA_DBCKEND) == 0) {
                        if (tmp_reg & SDIO_STA_DCRCFAIL) {
                            err = SDIO_EDCRCFAIL;
                        } else if (tmp_reg & SDIO_STA_RXOVERR) {
                            err = SDIO_ERXOVERR;
                        } else {
                            err = SDIO_EUNKNOWN; // XXX: unknown error
                        }
                    }
                    if (! err) {
                        for (ndx = 0; ndx < 2; ndx++) {
                            byte_swap(data_buf[ndx]);
                            c->scr[ndx] = data_buf[ndx];
                        }
                    }
                }
            }
        }
    }
    (void) sdio_select(0);
    if (err)
        kprintf("[SDIO] %s\n", stm32_sdio_errmsg(err));
    return err;
}

/*
 * Read a Block from our Card
 *
 * NB: There is a possibly useless test in this code, during the read
 * phase it allows that the SDIO card might try to send more than 512
 * bytes (128 32 bit longs) and allows it to do so, constantly over
 * writing the last long in the just-in-case-over-long-by-1 data buffer.
 * To compromise the system you would need a borked or custom crafted
 * sdio card which did that.
 */

int sdio_block_read(struct fnode *fno, void *_buf, uint32_t lba, int offset, int count)
{
    int err;
    uint32_t tmp_reg;
    uint32_t addr = lba;
    uint8_t *t;
    int ndx, bdx = 0;
    struct dev_sd *sdio;
    SDIO_CARD c;
    uint8_t *buf = _buf;
        
    
    sdio = (struct dev_sd *)FNO_MOD_PRIV(fno, &mod_sdio);
    if (!sdio)
        return -1;
    c = sdio->card;

    if (! SDIO_CARD_CCS(c)) {
        addr = lba * 512; // non HC cards use byte address
    }
    err = sdio_select(c->rca);
    if (! err) {
        err = stm32_sdio_command(16, 512);
        if (!err) {
            SDIO_DTIMER = 0xffffffff;
            SDIO_DLEN = 512;
            SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_9 |
                         SDIO_DCTRL_DTDIR |
                         SDIO_DCTRL_DTEN;
            err = stm32_sdio_command(17, addr);
            if (! err) {
                data_len = 0;
                do {
                    tmp_reg = SDIO_STA;
                    if (tmp_reg & SDIO_STA_RXDAVL) {
                        data_buf[data_len] = SDIO_FIFO;
                        if (data_len < 128) {
                            ++data_len;
                        }
                    }
                } while (tmp_reg & SDIO_STA_RXACT);
                if ((tmp_reg & SDIO_STA_DBCKEND) == 0) {
                    if (tmp_reg & SDIO_STA_DCRCFAIL) {
                        err = SDIO_EDCRCFAIL;
                    } else if (tmp_reg & SDIO_STA_RXOVERR) {
                        err = SDIO_ERXOVERR;
                    } else {
                        err = SDIO_EUNKNOWN; // Unknown Error!
                    }
                } else {
                    t = (uint8_t *)(data_buf);
                    /* copy out to the user buffer */
                    for (ndx = offset; ndx < (offset + count); ndx ++) {
                        buf[bdx++] = t[ndx];
                    }
                }
            }
        }
    }
    // deselect the card
    (void) sdio_select(0);
    if (err)
        kprintf("[SDIO] %s\n", stm32_sdio_errmsg(err));
    return err;
}

/*
 * Write a Block from our Card
 */
int
sdio_writeblock(SDIO_CARD c, uint32_t lba, uint8_t *buf) {
    int err;
    uint32_t tmp_reg;
    uint32_t addr = lba;
    uint8_t *t;
    int ndx;

    if (! SDIO_CARD_CCS(c)) {
        addr = lba * 512; // non HC cards use byte address
    }
    
    /*
     * Copy buffer to our word aligned buffer. Nominally you
     * can just use the passed in buffer and cast it to a
     * uint32_t * but that can cause issues if it isn't 
     * aligned.
     */
    t = (uint8_t *)(data_buf);
    for (ndx = 0; ndx < 512; ndx ++) {
        *t = *buf;
        buf++;
        t++;
    }
    err = sdio_select(c->rca);
    if (! err) {
        /* Set Block Size to 512 */
        err = stm32_sdio_command(16, 512);
        if (!err) {
            SDIO_DTIMER = 0xffffffff;
            SDIO_DLEN = 512;
            SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_9 |
                         SDIO_DCTRL_DTEN;
            err = stm32_sdio_command(24, addr);
            if (! err) {
                data_len = 0;
                do {
                    tmp_reg = SDIO_STA;
                    if (tmp_reg & SDIO_STA_TXFIFOHE) {
                        SDIO_FIFO = data_buf[data_len];
                        if (data_len < 128) {
                            ++data_len;
                        }
                    }
                } while (tmp_reg & SDIO_STA_TXACT);
                if ((tmp_reg & SDIO_STA_DBCKEND) == 0) {
                    if (tmp_reg & SDIO_STA_DCRCFAIL) {
                        err = SDIO_EDCRCFAIL;
                    } else if (tmp_reg & SDIO_STA_TXUNDERR) {
                        err = SDIO_ETXUNDER;
                    } else {
                        err = SDIO_EUNKNOWN; // Unknown Error!
                    }
                }
            }
        }
    }
    // deselect the card
    (void) sdio_select(0);
    if (err)
        kprintf("[SDIO] %s\n", stm32_sdio_errmsg(err));
    return err;
}

/*
 * sdio-status - Get Card Status page
 *
 * This function fetches the SD Card Status page and
 * copies it into the CARD structure.
 */
int
sdio_status(SDIO_CARD c) {
    uint32_t tmp_reg;
    int ndx;
    int err;

    err = sdio_select(c->rca);
    if (! err) {
        err = stm32_sdio_command(16, 64);
        if (! err) {
            err = stm32_sdio_command(55, c->rca << 16);
            if (! err) {
                SDIO_DTIMER = 0xffffffff;
                SDIO_DLEN = 64;
                SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_6 |
                             SDIO_DCTRL_DTDIR |
                             SDIO_DCTRL_DTEN;
                /* ACMD13 - Send Status Reg */
                err = stm32_sdio_command(13, 0);
                if (! err) {
                    data_len = 0;
                    do {
                        tmp_reg = SDIO_STA;
                        if (tmp_reg & SDIO_STA_RXDAVL) {
                            data_buf[data_len] = SDIO_FIFO;
                            if (data_len < 128) {
                                ++data_len;
                            }
                        }
                    } while (tmp_reg & SDIO_STA_RXACT);
                    if ((tmp_reg & SDIO_STA_DBCKEND) == 0) {
                        if (tmp_reg & SDIO_STA_DCRCFAIL) {
                            err = SDIO_EDCRCFAIL;
                        } else if (tmp_reg & SDIO_STA_RXOVERR) {
                            err = SDIO_ERXOVERR;
                        } else {
                            err = SDIO_EUNKNOWN; // Unknown Error!
                        }
                    } else {
                        for (ndx = 0; ndx < 16; ndx++) {
                            byte_swap(data_buf[ndx]);
                            c->status[ndx] = data_buf[ndx];
                        }
                    }
                    (void) sdio_select(0);
                }
            }
        }
    }

    if (err)
        kprintf("[SDIO] %s\n", stm32_sdio_errmsg(err));
    return err;
}

static struct SDIO_CARD_DATA __sdio_card_data;
#define MAX_RETRIES 5

/*
 * stm32_sdio_open - Prepare to use SDIO card
 *
 * This function resets the SDIO bus and identifies the
 * card (if any) that is plugged in. If there is no card
 * present, or an error in figuring out what the card is
 * (for example its an old MMC card) the function returns
 * NULL. If it fails and you have logging enabled you can
 * look at the last few commands sent.
 */
SDIO_CARD
stm32_sdio_open(void) {
    int err;
    int i;
    uint8_t *t;
    uint32_t tmp_reg;
    SDIO_CARD res = &__sdio_card_data;

    // basically bset(0, __sdio_card_data)
    t = (uint8_t *) &__sdio_card_data;
    for (i = 0; i < (int) sizeof(__sdio_card_data); i++) {
        *t++ = 0;
    }
    stm32_sdio_reset(SDIO_POWER_ON);
    err = stm32_sdio_command(0, 0);
    if (!err) {
        err = stm32_sdio_command(8, 0x1aa);
        if (!err) {
            // Woot! We support CMD8 so we're a v2 card at least */
            tmp_reg = SDIO_RESP1;
            __sdio_card_data.props = 1;
            i = 0;
            err = stm32_sdio_command(5, 0);
            if (! err) {
                // It is an SDIO card which is unsupported!
                err = SDIO_EBADCARD;
                return NULL;
            }
            do {
                err = stm32_sdio_command(55, 0); // broadcast ACMD
                if (err) {
                    break;   // try again
                }
                // Testing Card Busy, Voltage match, and capacity
                err = stm32_sdio_command(41, 0xc0100000);
                if (err != -2) {            // Expect CCRCFAIL here
                    break;               // try again
                }
                tmp_reg = SDIO_RESP1; // what did the card send?
                if ((tmp_reg & 0x80000000) == 0) {
                    continue;               // still powering up
                }
                res->ocr = tmp_reg;           // Ok OCR is valid
                break;
            } while (++i < MAX_RETRIES);
            if (res->ocr) {
                err = stm32_sdio_command(2, 0);
                if (! err) {
                    res->cid[0] = SDIO_RESP1;
                    res->cid[1] = SDIO_RESP2;
                    res->cid[2] = SDIO_RESP3;
                    res->cid[3] = SDIO_RESP4;
                    err = stm32_sdio_command(3, 0);   // get the RCA
                    if (! err) {
                        tmp_reg = SDIO_RESP1;
                        res->rca = (tmp_reg >> 16) & 0xffff;
                        if (! res->rca) {
                            /*
                             * If the card says '0' tell it to pick
                             * we assume this will work because the
                             * previous send RCA worked and the card
                             * should be in the ident state if it is
                             * functioning correctly.
                             */
                            (void) stm32_sdio_command(3, 0); // try again
                            tmp_reg = SDIO_RESP1;
                            res->rca = (tmp_reg >> 16) & 0xffff;
                        }
                        err = stm32_sdio_command(9, res->rca << 16);
                        if (! err) {
                            res->csd[0] = SDIO_RESP1;
                            res->csd[1] = SDIO_RESP2;
                            res->csd[2] = SDIO_RESP3;
                            res->csd[3] = SDIO_RESP4;
                            err = sdio_scr(res); // Capture the SCR
                            if (! err) {
                                /* All SD Cards support 4 bit bus and 24Mhz */
                                err = sdio_select(res->rca);
                                if (! err) {
                                    err = stm32_sdio_command(55, res->rca << 16);
                                    if (! err) {
                                        err = stm32_sdio_command(6, 2);
                                        if (! err) {
                                            //XXX stm32_sdio_bus(4, SDIO_24MHZ);
                                            //Seems we have speed issues for now...
                                            stm32_sdio_bus(4, SDIO_12MHZ);
                                            (void) sdio_select(0);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /* Compute the size of the card based on fields in the CSD
     * block. There are two kinds, V1 or V2. 
     * In the V1 Case :
     *     Size = 1<<BLOCK_LEN * 1<<(MULT+2) * (C_SIZE+1) bytes.
     * In the V2 Case :
     *     Size = (C_SIZE + 1) * 512K bytes.
     * But for our structure we want the size in 512 byte "blocks"
     * since that is the addressing unit we're going to export so
     * we compute the size / 512 as the "size" for the structure.
     */

    if (! err) {
        res->size = 0;
        switch (SDIO_CSD_VERSION(res)) {
            case 0:
                tmp_reg = ((1 << (SDIO_CSD1_CSIZE_MULT(res) + 2)) *
                           ( 1 << SDIO_CSD1_RBLKLEN(res))) >> 9;
                res->size = tmp_reg * (SDIO_CSD1_CSIZE(res) + 1);
                break;
            case 1:
                res->size = (SDIO_CSD2_CSIZE(res)+1) << 10; 
                break;
            default:
                res->size = 0; // Bug if its not CSD V1 or V2
        }
    }

    if (err)
        kprintf("[SDIO] %s\n", stm32_sdio_errmsg(err));
    return (err == 0) ? res : NULL;
}

void stm32_sdio_card_detect(void *arg)
{
    struct fnode *dev = arg;
    char name[4] = "sd0";
    memset(&SdCard[0], 0, sizeof(struct dev_sd));
    SdCard[0].card = stm32_sdio_open();
    if (!SdCard[0].card) {
        return;
    }
    kprintf("Found SD card in microSD slot.\r\n");
    SdCard[0].dev = device_fno_init(&mod_sdio, name, dev, FL_BLK, &SdCard[0]);
}
    
void stm32_sdio_init(struct fnode * dev)
{
    SDIO_CARD card;
    memset(&mod_sdio, 0, sizeof(mod_sdio));
    kprintf("Successfully initialized SDIO module.\r\n");
    strcpy(mod_sdio.name,"sdio");

    //mod_sdio.ops.close = sdio_close;
    mod_sdio.ops.block_read = sdio_block_read;

    register_module(&mod_sdio);
    tasklet_add(stm32_sdio_card_detect, dev);
}
