/*
 *---------------------------------------------------------------------------
 *
 * Copyright (c) 2020 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.
 * and its licensors.By using this software you agree to the terms of the
 * associated Analog Devices Software License Agreement.
 *
 *---------------------------------------------------------------------------
 */

#ifndef HAL_PORT_SPECIFIC_H
#define HAL_PORT_SPECIFIC_H

#include <stdlib.h>
#include <stdint.h>

#include <drivers/general/adi_drivers_general.h>
#include <drivers/spi/adi_spi.h>

#include "adi_platform.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define HAL_ALIGNED_PRAGMA(n)       ADI_ALIGNED_PRAGMA(n)
#define HAL_ALIGNED_ATTRIBUTE(n)    ADI_ALIGNED_ATTRIBUTE(n)

#define DMA_BUFFER_ALIGN(var, alignBytes)   HAL_ALIGNED_PRAGMA(alignBytes) var HAL_ALIGNED_ATTRIBUTE(alignBytes)

/*! Minimum transaction size above which DMA is enabled for a SPI transaction. */
/*  Interrupt-based transactions are used when the transaction size is less than this threshold. */
#define MIN_SIZE_FOR_DMA            (16)

/*! Duration of an MDIO read, in microseconds. This is used to convert timeout */
/*  values expressed in milliseconds, to a number of iterations when polling.  */
#define ADI_HAL_MDIO_READ_DURATION  (125)

/*! Indicates if the host IRQ is edge or level sensitive. */
#define ADI_EDGE_SENSITIVE_IRQ      (1)

/*! If set, it will disable the RX_RDY interrupt if no buffers are available */
/*  in the Rx queue for receiving incoming frames. Otherwise incoming frames */
/*  received while Rx queue is empty will be dropped.                        */
/*  Note this applies to Generic SPI protocol. In OPEN Alliance SPI this     */
/*  situation is handled differently, by using the NORX bit.                 */
#define ADI_PAUSE_RX_IF_NO_BUFFERS  (0)

#ifdef __cplusplus
}
#endif

#endif /* HAL_PORT_SPECIFIC_H */