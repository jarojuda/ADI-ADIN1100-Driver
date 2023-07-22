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

#ifndef _BSP_CFG_H
#define _BSP_CFG_H

/*Onboard EEPROM*/
#undef SPI_EEPROM

/* GPIO MDIO, pins 54, 56*/
#define MDIO_PORT               ADI_GPIO_PORT1
#define MDIO_PIN                ADI_GPIO_PIN_4
#define MDC_PORT                ADI_GPIO_PORT1
#define MDC_PIN                 ADI_GPIO_PIN_2

/*GPIO PHY Interrupt, pin 47*/
#define PHY_IRQ_PORT            ADI_GPIO_PORT1
#define PHY_IRQ_PIN             ADI_GPIO_PIN_0

#define BSP_LED1_PORT           ADI_GPIO_PORT2
#define BSP_LED1_PIN            ADI_GPIO_PIN_3
#define BSP_LED2_PORT           ADI_GPIO_PORT2
#define BSP_LED2_PIN            ADI_GPIO_PIN_4
#define BSP_LED3_PORT           ADI_GPIO_PORT2
#define BSP_LED3_PIN            ADI_GPIO_PIN_5
#define BSP_LED4_PORT           ADI_GPIO_PORT2
#define BSP_LED4_PIN            ADI_GPIO_PIN_6

#endif /* BSP_CFG_H */