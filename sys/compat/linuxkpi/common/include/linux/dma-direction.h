/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) Lutz Bichler <Lutz.Bichler@gmail.com>
 */

#ifndef _LINUXKPI_LINUX_DMA_DIRECTION_H_
#define _LINUXKPI_LINUX_DMA_DIRECTION_H_

enum dma_data_direction {
        DMA_BIDIRECTIONAL = 0,
        DMA_TO_DEVICE = 1,
        DMA_FROM_DEVICE = 2,
        DMA_NONE = 3,
};

#endif /* _LINUXKPI_LINUX_DMA_DIRECTION_H_ */
