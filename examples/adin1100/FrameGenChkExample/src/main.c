/*
 *---------------------------------------------------------------------------
 *
 * Copyright (c) 2016 - 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.
 * and its licensors.By using this software you agree to the terms of the
 * associated Analog Devices Software License Agreement.
 *
 *
 *---------------------------------------------------------------------------
 */

#include <stdbool.h>
#include "adin1100.h"
#include "boardsupport.h"
#include <math.h>

#define ADIN1100_INIT_ITER      (5)

/* Number of frames to be sent by the frame generator */
#define FRAME_COUNT             (20000)

uint8_t phyDevMem[ADI_PHY_DEVICE_SIZE];

adi_phy_DriverConfig_t phyDrvConfig = {
    .addr       = 0,
    .pDevMem    = (void *)phyDevMem,
    .devMemSize = sizeof(phyDevMem),
    .enableIrq  = true,
};

volatile uint32_t lastLinkCheckTime = 0;

int main(void)
{
    adi_eth_Result_e                    result;
    uint32_t                            error;
    uint32_t                            status;
    adin1100_DeviceStruct_t             dev;
    adin1100_DeviceHandle_t             hDevice = &dev;
    uint16_t                            capabilities;
    uint16_t                            fcRxErrCnt;
    uint32_t                            fcFrameCnt;
    adi_phy_FrameChkErrorCounters_t     fcErrCnt;
    bool                                fgDone;

    /****** System Init *****/
    error = BSP_InitSystem();
    DEBUG_RESULT("BSP_InitSystem", error, 0);

    /****** Driver Init *****/
    /* If both the MCU and the ADIN1100 are reset simultaneously */
    /* using the RESET button on the board, the MCU may start    */
    /* scanning for ADIN1100 devices before the ADIN1100 has     */
    /* powered up. This is worse if PHY address is configured as */
    /* 0 (default configuration of the board).                   */
    /* This is taken care of by iterating more than once over    */
    /* the valid MDIO address space.                             */
    for (uint32_t i = 0; i < ADIN1100_INIT_ITER; i++)
    {
        for (uint32_t phyAddr = 0; phyAddr < 8; phyAddr++)
        {
            phyDrvConfig.addr = phyAddr;
            result = adin1100_Init(hDevice, &phyDrvConfig);
            if (ADI_ETH_SUCCESS == result)
            {
                DEBUG_MESSAGE("Found PHY device at address %d", phyAddr);
                result = adin1100_ReadIrqStatus(hDevice, &status);
                result = adin1100_GetCapabilities(hDevice, &capabilities);
                DEBUG_RESULT("adin1100_GetCapabilities", result, ADI_ETH_SUCCESS);

                if (capabilities & ADI_PHY_CAP_TX_HIGH_LEVEL)
                {
                    DEBUG_MESSAGE("  - Supports 2.4V Tx level");
                }
                else
                {
                    DEBUG_MESSAGE("  - Does not support 2.4V Tx level");
                }

                if (capabilities & ADI_PHY_CAP_PMA_LOCAL_LOOPBACK)
                {
                    DEBUG_MESSAGE("  - Supports PMA local loopback");
                }
                else
                {
                    DEBUG_MESSAGE("  - Does not support PMA local loopback");
                }

                break;
            }
        }
        if (ADI_ETH_SUCCESS == result)
        {
            break;
        }
    }

    DEBUG_RESULT("No PHY device found", result, ADI_ETH_SUCCESS);

    /* Exit software powerdown and attamept to bring up the link. */
    result = adin1100_ExitSoftwarePowerdown(hDevice);
    DEBUG_RESULT("adin1100_ExitSoftwarePowerdown", result, ADI_ETH_SUCCESS);

    /* Enable frame checker and select the correct source */
    result = adin1100_FrameChkEn(hDevice, true);
    DEBUG_RESULT("adin1100_FrameChkEn", result, ADI_ETH_SUCCESS);
    result = adin1100_FrameChkSourceSelect(hDevice, ADI_PHY_FRAME_CHK_SOURCE_PHY);
    DEBUG_RESULT("adin1100_FrameChkSourceSelect", result, ADI_ETH_SUCCESS);

    /* Reset counters by reading RX_ERR_CNT registers */
    result = adin1100_FrameChkReadRxErrCnt(hDevice, &fcRxErrCnt);
    DEBUG_RESULT("adin1100_FrameChkReadRxErrCnt", result, ADI_ETH_SUCCESS);

    /* Set PCS loopback mode, causing all generated frames */
    /* to be looped backfor checking by the frame checker. */
    result = adin1100_SetLoopbackMode(hDevice, ADI_PHY_LOOPBACK_PCS);
    DEBUG_RESULT("adin1100_SetLoopbackMode", result, ADI_ETH_SUCCESS);

    /* Enable frame generator and configure burst mode, then start generating frames */
    result = adin1100_FrameGenEn(hDevice, true);
    DEBUG_RESULT("adin1100_FrameGenEn", result, ADI_ETH_SUCCESS);
    result = adin1100_FrameGenSetMode(hDevice, ADI_PHY_FRAME_GEN_MODE_BURST);
    DEBUG_RESULT("adin1100_FrameGenSetMode", result, ADI_ETH_SUCCESS);
    result = adin1100_FrameGenSetFrameCnt(hDevice, FRAME_COUNT);
    DEBUG_RESULT("adin1100_FrameGenSetFrameCnt", result, ADI_ETH_SUCCESS);
    result = adin1100_FrameGenRestart(hDevice);
    DEBUG_RESULT("adin1100_FrameGenRestart", result, ADI_ETH_SUCCESS);

    /* Wait for indication that all frames were generated */
    do
    {
        result = adin1100_FrameGenDone(hDevice, &fgDone);
        DEBUG_RESULT("adin1100_FrameGenDone", result, ADI_ETH_SUCCESS);
    } while ((result != ADI_ETH_SUCCESS) || !fgDone);

    /* Read results, note RX_ERR_CNT is read first to latch all counter values. */
    /* This is required before reading any of the other counter registers.      */
    result = adin1100_FrameChkReadRxErrCnt(hDevice, &fcRxErrCnt);
    DEBUG_RESULT("adin1100_FrameChkReadRxErrCnt", result, ADI_ETH_SUCCESS);
    result = adin1100_FrameChkReadFrameCnt(hDevice, &fcFrameCnt);
    DEBUG_RESULT("adin1100_FrameChkReadFrameCnt", result, ADI_ETH_SUCCESS);
    result = adin1100_FrameChkReadErrorCnt(hDevice, &fcErrCnt);
    DEBUG_RESULT("adin1100_FrameChkReadErrorCnt", result, ADI_ETH_SUCCESS);

    DEBUG_MESSAGE("Frame Generator/Checker Results:");
    DEBUG_MESSAGE("    (FG) Sent:           %d frames", FRAME_COUNT);
    DEBUG_MESSAGE("    (FC) Received:       %d frames", fcFrameCnt);
    DEBUG_MESSAGE("         Rx Errors             = %d", fcRxErrCnt);
    DEBUG_MESSAGE("           LEN_ERR_CNT         = %d", fcErrCnt.LEN_ERR_CNT);
    DEBUG_MESSAGE("           ALGN_ERR_CNT        = %d", fcErrCnt.ALGN_ERR_CNT);
    DEBUG_MESSAGE("           SYMB_ERR_CNT        = %d", fcErrCnt.SYMB_ERR_CNT);
    DEBUG_MESSAGE("           OSZ_CNT             = %d", fcErrCnt.OSZ_CNT);
    DEBUG_MESSAGE("           USZ_CNT             = %d", fcErrCnt.USZ_CNT);
    DEBUG_MESSAGE("           ODD_CNT             = %d", fcErrCnt.ODD_CNT);
    DEBUG_MESSAGE("           ODD_PRE_CNT         = %d", fcErrCnt.ODD_PRE_CNT);
    DEBUG_MESSAGE("           FALSE_CARRIER_CNT   = %d", fcErrCnt.FALSE_CARRIER_CNT);

    result = adin1100_FrameGenEn(hDevice, false);
    DEBUG_RESULT("adin1100_FrameGenEn", result, ADI_ETH_SUCCESS);

    result = adin1100_UnInit(hDevice);
    DEBUG_RESULT("adin1100_UnInit", result, ADI_ETH_SUCCESS);
}


