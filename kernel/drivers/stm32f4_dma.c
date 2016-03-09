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
#include <stdint.h>
#include "cirbuf.h"
#include <libopencm3/stm32/dma.h>

#include "stm32f4_dma.h"


void init_dma(const struct dma_setup * dma, uint32_t ptr, uint32_t len)
{
    dma_stream_reset(dma->base, dma->stream);

    dma_set_transfer_mode(dma->base, dma->stream, dma->dirn);
    dma_set_priority(dma->base, dma->stream, dma->prio);

    dma_set_peripheral_address(dma->base, dma->stream, dma->paddr);
    dma_disable_peripheral_increment_mode(dma->base, dma->stream);
    dma_set_peripheral_size(dma->base, dma->stream, dma->psize);

    dma_enable_memory_increment_mode(dma->base, dma->stream);
    dma_set_memory_size(dma->base, dma->stream, dma->msize);

    dma_enable_direct_mode(dma->base, dma->stream);
    dma_set_dma_flow_control(dma->base, dma->stream);

    dma_channel_select(dma->base,dma->stream,dma->channel);

    dma_set_memory_address(dma->base, dma->stream, ptr);
    dma_set_number_of_data(dma->base, dma->stream, len);
    dma_enable_stream(dma->base, dma->stream);
}
