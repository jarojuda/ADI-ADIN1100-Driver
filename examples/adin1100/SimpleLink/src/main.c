/*
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

#include <stdbool.h>
#include "adin1100.h"
#include "boardsupport.h"
#include <math.h>
#define ADIN1100_INIT_ITER      (5)

uint8_t phyDevMem[ADI_PHY_DEVICE_SIZE];

adi_phy_DriverConfig_t phyDrvConfig = {
    .addr       = 0,
    .pDevMem    = (void *)phyDevMem,
    .devMemSize = sizeof(phyDevMem),
    .enableIrq  = true,
};

volatile uint32_t lastLinkCheckTime = 0;
volatile bool     irqFired = false;

/* Static configuration function for the ADIN1100, it is executed after */
/* reset and initialization, with the ADIN1100 in software powerdown.   */
adi_eth_Result_e appPhyConfig(adin1100_DeviceHandle_t hDevice)
{
    adi_eth_Result_e        result = ADI_ETH_SUCCESS;

    /* Any static configuration that needs to be done before the link */
    /* is brought up can be included here.                            */
    result = adin1100_AnAdvTxMode(hDevice, ADI_PHY_AN_ADV_TX_REQ_2P4V);

    return result;
}

void phyCallback(void *pCBParam, uint32_t Event, void *pArg)
{
    irqFired = true;
}



int main(void)
{
    uint32_t                run = 1;
    adi_phy_LinkStatus_e    linkstatus = ADI_PHY_LINK_STATUS_DOWN;
    adi_phy_LinkStatus_e    oldlinkstatus = ADI_PHY_LINK_STATUS_DOWN;
    adi_phy_AnStatus_t      anstatus;
    char                    *strBuffer;
    adi_eth_Result_e        result;
    uint32_t                error;
    uint32_t                status;
    adin1100_DeviceStruct_t dev;
    adin1100_DeviceHandle_t hDevice = &dev;
    uint16_t                capabilities;

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

    /* Device configuration, performed while the device is in software powerdown. */
    result = appPhyConfig(hDevice);
    DEBUG_RESULT("appPhyConfig", result, ADI_ETH_SUCCESS);

    /* Register callback for selected events. Note that the interrupts corresponding to */
    /* ADI_PHY_EVT_HW_RESET and ADI_PHY_EVT_CRSM_HW_ERROR are always unmasked so they   */
    /* can be ommitted from the argument, here they are explicit for clarity.           */
    result = adin1100_RegisterCallback(hDevice, phyCallback, ADI_PHY_EVT_HW_RESET | ADI_PHY_EVT_CRSM_HW_ERROR | ADI_PHY_EVT_LINK_STAT_CHANGE);
    DEBUG_RESULT("adin1100_RegisterCallback", result, ADI_ETH_SUCCESS);

    /* Exit software powerdown and attamept to bring up the link. */
    result = adin1100_ExitSoftwarePowerdown(hDevice);
    DEBUG_RESULT("adin1100_ExitSoftwarePowerdown", result, ADI_ETH_SUCCESS);

    /* The link may not be up at this point */
    result = adin1100_GetLinkStatus(hDevice, &linkstatus);
    DEBUG_RESULT("adin1100_GetLinkStatus", result, ADI_ETH_SUCCESS);
    while (run)
    {
        uint32_t now  = BSP_SysNow();

        /* Check the link status every 250ms approximately */
        if (now - lastLinkCheckTime >= 250)
        {
            lastLinkCheckTime = now;

            if (irqFired)
            {
                irqFired = false;

                result = adin1100_ReadIrqStatus(hDevice, &status);
                DEBUG_RESULT("adin1100_ReadIrqStatus", result, ADI_ETH_SUCCESS);

                if (status & ADI_PHY_EVT_HW_RESET)
                {
                    result = adin1100_ReInitPhy(hDevice);
                    DEBUG_RESULT("adin1100_ReInitPhy", result, ADI_ETH_SUCCESS);

                    result = appPhyConfig(hDevice);
                    DEBUG_RESULT("appPhyConfig", result, ADI_ETH_SUCCESS);

                    result = adin1100_ExitSoftwarePowerdown(hDevice);
                    DEBUG_RESULT("adin1100_ExitSoftwarePowerdown", result, ADI_ETH_SUCCESS);
                }
                if (status & ADI_PHY_EVT_LINK_STAT_CHANGE)
                {
                    DEBUG_MESSAGE("INT: Link status changed");
                }
            }

            result = adin1100_GetLinkStatus(hDevice, &linkstatus);
            if (result == ADI_ETH_COMM_ERROR)
            {
                DEBUG_MESSAGE("MDIO communication error");
            }
            else
            {
                DEBUG_RESULT("adin1100_GetLinkStatus", result, ADI_ETH_SUCCESS);

                if (oldlinkstatus != linkstatus)
                {
                    switch (linkstatus)
                    {
                        case ADI_PHY_LINK_STATUS_UP:
                            result = adin1100_GetAnStatus(hDevice, &anstatus);
                            if (result == ADI_ETH_COMM_ERROR)
                            {
                              DEBUG_MESSAGE("MDIO communication error");
                            }

                            strBuffer = "Link Up";
                            DEBUG_MESSAGE(strBuffer);
                            switch(anstatus.anMsResolution)
                            {
                              case ADI_PHY_AN_MS_RESOLUTION_SLAVE:
                                strBuffer = "M/S: Master";
                              break;
                              case ADI_PHY_AN_MS_RESOLUTION_MASTER:
                                   strBuffer= "M/S: Slave";
                              break;
                            }
                            DEBUG_MESSAGE(strBuffer);
                            switch(anstatus.anTxMode)
                            {
                              case ADI_PHY_AN_TX_LEVEL_1P0V:
                                   strBuffer = "Tx Level: 1.0V";
                              break;
                              case ADI_PHY_AN_TX_LEVEL_2P4V:
                                   strBuffer = "Tx Level: 2.4V";
                              break;
                            }
                            DEBUG_MESSAGE(strBuffer);
                            BSP_ErrorLed(true);
                            break;

                        case ADI_PHY_LINK_STATUS_DOWN:
                            strBuffer = "Link Down";
                            DEBUG_MESSAGE(strBuffer);
                            BSP_ErrorLed(false);
                            break;

                        default:
                            DEBUG_RESULT("Invalid link status", 0, 1);
                    }

                }
                oldlinkstatus = linkstatus;
            }
            BSP_HeartBeat();
        }
    }
    result = adin1100_UnInit(hDevice);
}


