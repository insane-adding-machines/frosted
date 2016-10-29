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
 *      Authors:
 *
 */
 
#include "frosted.h"
#include "device.h"
#include "cirbuf.h"
#include <stdint.h>
#include "uart_dev.h"
#include "uart.h"
#include "poll.h"

#include "unicore-mx/cm3/nvic.h"

#ifdef LM3S
#   include "unicore-mx/lm3s/usart.h"
#   define CLOCK_ENABLE(C)
#   define USART_SR_RXNE  USART_IC_RX
#   define USART_SR_TXE   USART_IC_TX
#endif
#ifdef STM32F4
#   include "unicore-mx/stm32/usart.h"
#   include "unicore-mx/stm32/rcc.h"
#   define CLOCK_ENABLE(C)                 rcc_periph_clock_enable(C);
#   define usart_clear_rx_interrupt(x) do{}while(0)
#   define usart_clear_tx_interrupt(x) do{}while(0)
#endif
#ifdef STM32F7
#   include "unicore-mx/stm32/usart.h"
#   include "unicore-mx/stm32/rcc.h"
#   define CLOCK_ENABLE(C)                 rcc_periph_clock_enable(C);
#   define usart_clear_rx_interrupt(x) do{}while(0)
#   define usart_clear_tx_interrupt(x) do{}while(0)
#endif
#ifdef LPC17XX
#   include "unicore-mx/lpc17xx/uart.h"
#   include "unicore-mx/lpc17xx/pwr.h"
#   define CLOCK_ENABLE(C) pwr_enable_peripherals(C)
#endif

struct dev_uart {
    struct device * dev;
    uint32_t base;
    struct cirbuf *inbuf;
    struct cirbuf *outbuf;
    uint16_t sid;
    uint8_t *w_start;
    uint8_t *w_end;
};

#define MAX_UARTS 8
static struct dev_uart *DEV_UART[MAX_UARTS] = { };

static int devuart_write(struct fnode *fno, const void *buf, unsigned int len);
static int devuart_read(struct fnode *fno, void *buf, unsigned int len);
static int devuart_poll(struct fnode *fno, uint16_t events, uint16_t *revents);
static void devuart_tty_attach(struct fnode *fno, int pid);

static struct module mod_devuart = {
    .family = FAMILY_FILE,
    .name = "uart",
    .ops.open = device_open,
    .ops.read = devuart_read,
    .ops.poll = devuart_poll,
    .ops.write = devuart_write,
    .ops.tty_attach = devuart_tty_attach,
};

static void uart_send_break(void *arg)
{
    int *pid = (int *)(arg);
    if (pid)
        task_kill(*pid, 2);
}

static void devuart_tty_attach(struct fnode *fno, int pid)
{
    struct dev_uart *uart = (struct dev_uart *)FNO_MOD_PRIV(fno, &mod_devuart);
    if (uart->sid != pid) {
        //kprintf("/dev/%s active job pid: %d\r\n", fno->fname, pid);
        uart->sid = pid;
    }
}


void uart_isr(struct dev_uart *uart)
{
    if (!uart)
        return;
    /* TX interrupt */
    if (usart_get_interrupt_source(uart->base, USART_SR_TXE)) {
        usart_clear_tx_interrupt(uart->base);

//        if (this_task() != NULL) /* cannot spinlock in ISR! */
//            mutex_lock(uart->dev->mutex);

        /* Are there bytes left to be written? */
        if (cirbuf_bytesinuse(uart->outbuf))
        {
            uint8_t outbyte;
            cirbuf_readbyte(uart->outbuf, &outbyte);
            usart_send(uart->base, (uint16_t)(outbyte));
        } else {
            usart_disable_tx_interrupt(uart->base);
            /* If a process is attached, resume the process */
            if (uart->dev->task != NULL)
                task_resume(uart->dev->task);
        }
//        mutex_unlock(uart->dev->mutex);
    }

    /* RX interrupt */
    if (usart_get_interrupt_source(uart->base, USART_SR_RXNE)) {
        usart_clear_rx_interrupt(uart->base);
        /* if data available */
        if (usart_is_recv_ready(uart->base))
        {
            char byte = (char)(usart_recv(uart->base) & 0xFF);

            /* Intercept ^C */
            if (byte == 3) {
                if (uart->sid > 1) {
                    tasklet_add(uart_send_break, &uart->sid);
                }
                return;
            }
            /* read data into circular buffer */
            cirbuf_writebyte(uart->inbuf, byte);
        }
        /* If a process is attached, resume the process */
        if (uart->dev->task != NULL)
            task_resume(uart->dev->task);
    }

}

void uart0_isr(void)
{
    uart_isr(DEV_UART[0]);
}

void uart1_isr(void)
{
    uart_isr(DEV_UART[1]);
}

void uart2_isr(void)
{
    uart_isr(DEV_UART[2]);
}

#ifdef USART0
void usart0_isr(void)
{
    uart_isr(DEV_UART[0]);
}
#endif

#ifdef USART1
void usart1_isr(void)
{
    uart_isr(DEV_UART[1]);
}
#endif
#ifdef USART2
void usart2_isr(void)
{
    uart_isr(DEV_UART[2]);
}
#endif
#ifdef USART3
void usart3_isr(void)
{
    uart_isr(DEV_UART[3]);
}
#endif
#ifdef USART6
void usart6_isr(void)
{
    uart_isr(DEV_UART[6]);
}
#endif


static int devuart_write(struct fnode *fno, const void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;
    struct dev_uart *uart;
    
    uart = (struct dev_uart *)FNO_MOD_PRIV(fno, &mod_devuart);
    if (!uart)
        return -1;
        
    mutex_lock(uart->dev->mutex);
    if (cirbuf_bytesinuse(uart->outbuf) && usart_is_send_ready(uart->base)) {
        char c;
        cirbuf_readbyte(uart->outbuf, &c);
        usart_send(uart->base, (uint16_t) c);
        usart_enable_tx_interrupt(uart->base);
    }

    if (!cirbuf_bytesfree(uart->outbuf)) {
        mutex_unlock(uart->dev->mutex);
        task_preempt();
        return SYS_CALL_AGAIN;
    }

    if (len <= 0)
        return len;
    if (uart->w_start == NULL) {
        uart->w_start = (uint8_t *)buf;
        uart->w_end = ((uint8_t *)buf) + len;

    } else {
        /* previous transmit not finished, do not update w_start */
    }


    /* write to circular output buffer */
    uart->w_start += cirbuf_writebytes(uart->outbuf, uart->w_start, uart->w_end - uart->w_start);
    if (usart_is_send_ready(uart->base)) {
        char c;
        cirbuf_readbyte(uart->outbuf, &c);
        usart_send(uart->base, (uint16_t) c);
        usart_enable_tx_interrupt(uart->base);
    }

    if (cirbuf_bytesinuse(uart->outbuf) == 0) {
        mutex_unlock(uart->dev->mutex);
        usart_disable_tx_interrupt(uart->base);
        uart->w_start = NULL;
        uart->w_end = NULL;
        return len;
    }


    if (uart->w_start < uart->w_end)
    {
        uart->dev->task = this_task();
        mutex_unlock(uart->dev->mutex);
        task_suspend();
        return SYS_CALL_AGAIN;
    }

    mutex_unlock(uart->dev->mutex);
    uart->w_start = NULL;
    uart->w_end = NULL;
    return len;
}


static int devuart_read(struct fnode *fno, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    struct dev_uart *uart;

    if (len <= 0)
        return len;

    uart = (struct dev_uart *)FNO_MOD_PRIV(fno, &mod_devuart);
    if (!uart)
        return -1;

    mutex_lock(uart->dev->mutex);
    usart_disable_rx_interrupt(uart->base);
    len_available =  cirbuf_bytesinuse(uart->inbuf);
    if (len_available <= 0) {
        uart->dev->task = this_task();
        task_suspend();
        mutex_unlock(uart->dev->mutex);
        out = SYS_CALL_AGAIN;
        goto again;
    }

    if (len_available < len)
        len = len_available;

    for(out = 0; out < len; out++) {
        /* read data */
        if (cirbuf_readbyte(uart->inbuf, ptr) != 0)
            break;
        ptr++;
    }

again:
    usart_enable_rx_interrupt(uart->base);
    mutex_unlock(uart->dev->mutex);
    return out;
}


static int devuart_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    int ret = 0;
    struct dev_uart *uart;

    uart = (struct dev_uart *)FNO_MOD_PRIV(fno, &mod_devuart);
    if (!uart)
        return -1;

    uart->dev->task = this_task();
    mutex_lock(uart->dev->mutex);
    usart_disable_rx_interrupt(uart->base);
    *revents = 0;
    if ((events & POLLOUT) && (cirbuf_bytesfree(uart->outbuf) > 0)) {
        *revents |= POLLOUT;
        ret = 1;
    }
    if ((events == POLLIN) && (cirbuf_bytesinuse(uart->inbuf) > 0)) {
        *revents |= POLLIN;
        ret = 1;
    }
    usart_enable_rx_interrupt(uart->base);
    mutex_unlock(uart->dev->mutex);
    return ret;
}

static int uart_fno_init(const struct uart_config * addr)
{
    static int num_ttys = 0;
    char name[6] = "ttyS";
    struct fnode *devfs = fno_search("/dev");
    struct dev_uart *u;
    if (!devfs)
        return -ENOENT;

    u = kalloc(sizeof(struct dev_uart));
    if (!u)
        return -ENOMEM;
    memset(u, 0, sizeof(struct dev_uart));

    name[4] =  '0' + num_ttys++;
    u->base = addr->base;
    u->dev = device_fno_init(&mod_devuart, name, devfs, FL_TTY, u);
    u->inbuf = cirbuf_create(256);
    u->outbuf = cirbuf_create(256);
    u->dev->task = NULL;
    DEV_UART[addr->devidx] = u;
    return 0;

}

int uart_create(const struct uart_config *uart)
{
    if (uart->base == 0)
        return -EINVAL;

    gpio_create(&mod_devuart, &uart->pio_rx);
    gpio_create(&mod_devuart, &uart->pio_tx);

    uart_fno_init(uart);
    CLOCK_ENABLE(uart->rcc);
    usart_enable_rx_interrupt(uart->base);
    usart_set_baudrate(uart->base, uart->baudrate);
    usart_set_databits(uart->base, uart->data_bits);
    usart_set_stopbits(uart->base, uart->stop_bits);
    usart_set_mode(uart->base, USART_MODE_TX_RX);
    usart_set_parity(uart->base, uart->parity);
    usart_set_flow_control(uart->base, uart->flow);
    usart_enable_rx_interrupt(uart->base);
    nvic_enable_irq(uart->irq);
    usart_enable(uart->base);
}

int uart_init(void)
{
    register_module(&mod_devuart);
    return 0;
}
