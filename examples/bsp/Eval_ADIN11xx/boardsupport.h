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

#ifndef BOARDSUPPORT_H
#define BOARDSUPPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "bsp_config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define HAL_ALIGNED_PRAGMA(n)       ADI_ALIGNED_PRAGMA(n)
#define HAL_ALIGNED_ATTRIBUTE(n)    ADI_ALIGNED_ATTRIBUTE(n)

#if defined(ADIN1100) || defined(ADIN1110)
#define SPI_DEVICE_0    0u
/* SPI0 device, desired clock  */
#define SPI_CLK_DEVICE_0    13000000
#endif

/*Onboard EEPROM*/
#ifdef SPI_EEPROM
#define SPI_DEVICE_1    1u
#endif

/*MDIO BSP*/
/* MDIO Interface SPI2*/
#define SPI_DEVICE_2    2u
/*  MDIO SPI2*/
#define SPI_CLK_MDIO   2500000
/** define size of data buffers */
#define SPI_BUFFERSIZE 1600

/*Uart BSP*/
#define UART_DEVICE_NUM 0u

#define TICKS        (26000000u / 1000u)
#define NUM_SYSTICKS (50u)
#define TIMEOUT_VAL  (1000000u)


#define ADUCM4050_SYSTEM_SPEED_HZ       (26000000u)
#define ADUCM4050_TIMER_SPEED_10HZ      (40625)

/* Buffer for debug messages written to UART. */
extern char aDebugString[150u];

void common_Fail(char *FailureReason);
void common_Perf(char *InfoString);

#define DEBUG_MESSAGE(...) \
  do { \
    sprintf(aDebugString,__VA_ARGS__); \
    common_Perf(aDebugString); \
  } while(0)

#define DEBUG_RESULT(s,result,expected_value) \
  do { \
    if ((result) != (expected_value)) { \
      sprintf(aDebugString,"%s  %d", __FILE__,__LINE__); \
      common_Fail(aDebugString); \
      sprintf(aDebugString,"%s Error Code: 0x%08X\n\rFailed\n\r",(s),(result)); \
      common_Perf(aDebugString); \
      exit(0); \
    } \
  } while (0)

/*
* Driver Version
*/
typedef struct _ADUCM4050_LIB_VERSION
{
  uint16_t api;        /*!< API version */
  uint16_t sil;        /*!<  Silicon version*/
} ADUCM4050_LIB_VERSION;

typedef void (* ADI_CB) (  /*!< Callback function pointer */
    void      *pCBParam,         /*!< Client supplied callback param */
    uint32_t   Event,            /*!< Event ID specific to the Driver/Service */
    void      *pArg);            /*!< Pointer to the event specific argument */

/*Functions prototypes*/
uint32_t BSP_InitSystem(void);
uint32_t BSP_SysNow();
uint32_t BSP_RegisterIRQCallback(ADI_CB const *intCallback, void * hDevice);
void BSP_DisableIRQ(void);
void BSP_EnableIRQ(void);
uint32_t BSP_SetPinMDC(bool set);
uint32_t BSP_SetPinMDIO(bool set);
uint16_t BSP_GetPinMDInput(void);
void BSP_ChangeMDIPinDir(bool output);
uint32_t BSP_spi0_write_and_read(uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes, bool useDma);
uint32_t BSP_spi0_register_callback(ADI_CB const *pfCallback, void *const pCBParam);
uint32_t BSP_spi2_write_and_read(uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes);

void        BSP_HWReset             (bool set);
void        BSP_HeartBeat           (void);
void        BSP_ErrorLed            (bool on);
void        BSP_FuncLed1            (bool on);
void        BSP_FuncLed1Toggle      (void);
void        BSP_FuncLed2            (bool on);
void        BSP_FuncLed2Toggle      (void);
void        BSP_LedToggleAll        (void);

extern void msgWrite(char * ptr);

#ifdef __cplusplus
}
#endif

#endif /* BOARDSUPPORT_H */
