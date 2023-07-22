/*
 *  $Id: pinmux.c $
 *  $Revision: 1.0 $
 *  $Date: 2020-01-21 $
 *
 *---------------------------------------------------------------------------
 *
 * Copyright (c) 2016 - 2020 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. 
 * and its licensors.By using this software you agree to the terms of the 
 * associated Analog Devices Software License Agreement.
 *
 *
 *---------------------------------------------------------------------------
 */

#include <sys/platform.h>
#include <stdint.h>
#include "bsp_config.h"

#define SPI0_CLK_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<0))
#define SPI0_MOSI_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<2))
#define SPI0_MISO_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<4))
#define SPI0_CS_0_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<6))
#define SPI1_CLK_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<12))
#define SPI1_MOSI_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<14))
#define SPI1_MISO_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<16))
#define SPI1_CS_0_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<18))
#define SPI1_CS_1_PORTP2_MUX  ((uint32_t) ((uint32_t) 1<<22))
#define UART0_TX_PORTP0_MUX  ((uint32_t) ((uint32_t) 1<<20))
#define UART0_RX_PORTP0_MUX  ((uint32_t) ((uint32_t) 1<<22))
#define SPI1_CS_3_PORTP2_MUX  ((uint32_t) ((uint32_t) 2<<30))
   
#ifndef MDIO_GPIO   
#define SPI2_CLK_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<4))
#define SPI2_MOSI_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<6))
#define SPI2_MISO_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<8))
#define SPI2_CS_0_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<10))
#endif
int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) 
{
    *pREG_GPIO0_CFG = SPI0_CLK_PORTP0_MUX | SPI0_MOSI_PORTP0_MUX
     | SPI0_MISO_PORTP0_MUX | SPI0_CS_0_PORTP0_MUX | UART0_TX_PORTP0_MUX
     | UART0_RX_PORTP0_MUX;

    *pREG_GPIO2_CFG = SPI1_CS_3_PORTP2_MUX;

#ifndef MDIO_GPIO      
     *pREG_GPIO1_CFG = SPI1_CLK_PORTP1_MUX | SPI1_MOSI_PORTP1_MUX
     | SPI1_MISO_PORTP1_MUX | SPI2_CLK_PORTP1_MUX | SPI2_MOSI_PORTP1_MUX
     | SPI2_MISO_PORTP1_MUX | SPI2_CS_0_PORTP1_MUX;
#else
    
     *pREG_GPIO1_CFG = SPI1_CLK_PORTP1_MUX | SPI1_MOSI_PORTP1_MUX
     | SPI1_MISO_PORTP1_MUX;
     
#endif
     return 0;
}

