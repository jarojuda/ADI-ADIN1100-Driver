/*
 *---------------------------------------------------------------------------
 *
 * Copyright (c) 2020 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary <and confidential> to Analog Devices, Inc.
 * and its licensors.By using this software you agree to the terms of the
 * associated Analog Devices Software License Agreement.
 *
 *---------------------------------------------------------------------------
 */

#ifndef ADI_PLATFORM_H
#define ADI_PLATFORM_H

#include <stdint.h>
#include "boardsupport.h"
#include "ADIN1100_addr_rdef_22.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint32_t adi_MdioRead           (uint8_t phyAddr, uint8_t phyReg, uint16_t *phyData );
uint32_t adi_MdioWrite          (uint8_t phyAddr, uint8_t phyReg, uint16_t phyData );
#if defined(MDIO_CL22)
uint32_t adi_MdioRead_Cl22(uint8_t phyAddr, uint32_t phyReg, uint16_t *phyData );
uint32_t adi_MdioWrite_Cl22(uint8_t phyAddr, uint32_t phyReg, uint16_t phyData );
#else
uint32_t adi_MdioRead_Cl45      (uint8_t phyAddr, uint32_t phyReg, uint16_t *phyData );
uint32_t adi_MdioWrite_Cl45     (uint8_t phyAddr, uint32_t phyReg, uint16_t phyData );
#endif
uint32_t  ADI_RegisterIRQCallback(ADI_CB const *intCallback, void * hDevice);

#ifdef __cplusplus
}
#endif

#endif /* ADI_PLATFORM_H */