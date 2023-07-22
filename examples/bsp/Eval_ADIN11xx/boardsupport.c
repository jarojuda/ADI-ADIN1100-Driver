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

#include <drivers/general/adi_drivers_general.h>
#include <drivers/pwr/adi_pwr.h>
#include <drivers/gpio/adi_gpio.h>
#include <drivers/spi/adi_spi.h>
#include <drivers/tmr/adi_tmr.h>
#include <drivers/uart/adi_uart.h>
#include <ctype.h>

#include <drivers/xint/adi_xint.h>
#include <adi_processor.h>

#include "adi_common.h"

#include "boardsupport.h"
#include "bsp_config.h"



/*GPIO */
uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE];


/*SPI */
#ifdef ADIN1110
ADI_SPI_HANDLE hSpi0;
HAL_ALIGNED_PRAGMA(4)
static uint8_t spidevicememSPI0[ADI_SPI_MEMORY_SIZE] HAL_ALIGNED_ATTRIBUTE(4);
#endif

#ifdef SPI_EEPROM
ADI_SPI_HANDLE hSPI1;
HAL_ALIGNED_PRAGMA(4)
static uint8_t spidevicememSPI1[ADI_SPI_MEMORY_SIZE] HAL_ALIGNED_ATTRIBUTE(4);
#endif
#ifdef ADIN1100
ADI_SPI_HANDLE hSpi2;
HAL_ALIGNED_PRAGMA(4)
static uint8_t spidevicememSPI2[ADI_SPI_MEMORY_SIZE] HAL_ALIGNED_ATTRIBUTE(4);
#endif

/*UART */
/* Handle for the UART device. */
static ADI_UART_HANDLE hDeviceUart;

char aDebugString[150u];

/* Memory for the UART driver. */
/* Memory required by the driver for bidirectional mode of operation. */
#define ADI_UART_MEMORY_SIZE    (ADI_UART_BIDIR_MEMORY_SIZE)
ADI_ALIGNED_PRAGMA(4)
static uint8_t UartDeviceMem[ADI_UART_MEMORY_SIZE] ADI_ALIGNED_ATTRIBUTE(4);

#if defined(__ADUCM302x__)
#define GP_TMR_CAPTURE_EVENT 14u
#elif defined(__ADUCM4x50__)
#define GP_TMR_CAPTURE_EVENT 16u
#else
#error TMR is not ported for this processor
#endif

#define LOAD_VALUE_FOR_200MS_PERIOD (24063u)

static  volatile uint64_t debugTimer = 0;

/*Extern defined functions and variables*/
extern int32_t adi_initpinmux(void);


void BSP_ClrPendingIRQ(void)
{
    NVIC_ClearPendingIRQ(SYS_GPIO_INTA_IRQn);
}

void BSP_EnableIRQ(void)
{
    NVIC_EnableIRQ(SYS_GPIO_INTA_IRQn);
}
void BSP_DisableIRQ(void)
{
    NVIC_DisableIRQ(SYS_GPIO_INTA_IRQn);
}


/*
 * @brief GPIO manipulating
 *
 * @param [in]      set - true/false
 * @param [in]      Port - which GPIO port
 * @param [in]      Pins - which GPIO pin
 * @param [out]     none
 * @return none
 *
 * @details
 *
 * @sa
 */
void BSP_GpioSetPin(bool set, const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins)
{
    if (set)
    {
        adi_gpio_SetHigh(Port, Pins);
    }
    else
    {
        adi_gpio_SetLow(Port, Pins);
    }

}

/*
 * @brief Helper for MDIO GPIO Clock toggle
 *
 * @param [in]      set - true/false
 * @param [out]     none
 * @return 0
 *
 * @details
 *
 * @sa
 */
uint32_t BSP_SetPinMDC(bool set)
{
    if (set)
    {
        adi_gpio_SetHigh(MDC_PORT, MDC_PIN);
    }
    else
    {
        adi_gpio_SetLow(MDC_PORT, MDC_PIN);
    }
    return 0;
}


/*
 * @brief Helper for MDIO GPIO Read Pin value
 *
 * @param [in]      none
 * @param [out]     none
 * @return pin value
 *
 * @details
 *
 * @sa
 */
uint16_t BSP_GetPinMDInput(void)
{
  uint16_t data = 0;
  uint16_t val = 0;
  adi_gpio_GetData ( MDIO_PORT, MDIO_PIN, &data);
  if((data & MDIO_PIN) == MDIO_PIN)
  {
    val = 1;
  }
  else
  {
    val = 0;
  }

return val;
}

/*
 * @brief Helper for MDIO GPIO Changes pin in/out
 *
 * @param [in]      output - true/false
 * @param [out]     none
 * @return none
 *
 * @details
 *
 * @sa
 */
void BSP_ChangeMDIPinDir(bool output)
{
  if (output == true)
  {
    adi_gpio_OutputEnable(MDIO_PORT, MDIO_PIN, true);
    adi_gpio_InputEnable(MDIO_PORT, MDIO_PIN, false);

  }
  else
  {
    adi_gpio_OutputEnable(MDIO_PORT, MDIO_PIN, false);
    adi_gpio_InputEnable(MDIO_PORT, MDIO_PIN, true);

  }

}

/*
 * @brief Helper for MDIO GPIO Changes ouptut pin value
 *
 * @param [in]      output - true/false
 * @param [out]     none
 * @return 0
 *
 * @details
 *
 * @sa
 */
uint32_t BSP_SetPinMDIO(bool set)
{
    if (set)
    {
        adi_gpio_SetHigh(MDIO_PORT, MDIO_PIN);
    }
    else
    {
        adi_gpio_SetLow(MDIO_PORT, MDIO_PIN);
    }
    return 0;
}



void BSP_HWReset(bool set)
{
  if(set == true)
  {
    adi_gpio_SetHigh(ADI_GPIO_PORT0, ADI_GPIO_PIN_8);
  }
  else
  {
    adi_gpio_SetLow(ADI_GPIO_PORT0, ADI_GPIO_PIN_8);
  }
}


/* LED functions */

static void bspLedSet(const ADI_GPIO_PORT port, const ADI_GPIO_DATA pins, bool on)
{
    if (on)
    {
        adi_gpio_SetLow(port, pins);
    }
    else
    {
        adi_gpio_SetHigh(port, pins);
    }
}

static void bspLedToggle(const ADI_GPIO_PORT port, const ADI_GPIO_DATA pins)
{
      adi_gpio_Toggle(port, pins);
}


/*
 * Heartbeat LED, ORANGE
 */
void BSP_HeartBeat(void)
{
    bspLedToggle(BSP_LED3_PORT, BSP_LED3_PIN);
}


/*
 * Error LED, RED
 */
void BSP_ErrorLed(bool on)
{
    bspLedSet(BSP_LED2_PORT, BSP_LED2_PIN, on);
}

/*
 * Custom function 1 LED
 */
void BSP_FuncLed1(bool on)
{
    bspLedSet(BSP_LED1_PORT, BSP_LED1_PIN, on);
}

void BSP_FuncLed1Toggle(void)
{
    bspLedToggle(BSP_LED1_PORT, BSP_LED1_PIN);
}

/*
 * Custom function 2 LED
 */
void BSP_FuncLed2(bool on)
{
    bspLedSet(BSP_LED4_PORT, BSP_LED4_PIN, on);
}

void BSP_FuncLed2Toggle(void)
{
    bspLedToggle(BSP_LED4_PORT, BSP_LED4_PIN);
}

/* All LEDs toggle, used to indicate hardware failure on the board */
void BSP_LedToggleAll(void)
{
    bspLedToggle(BSP_LED1_PORT, BSP_LED1_PIN);
    bspLedToggle(BSP_LED2_PORT, BSP_LED2_PIN);
    bspLedToggle(BSP_LED3_PORT, BSP_LED3_PIN);
    bspLedToggle(BSP_LED4_PORT, BSP_LED4_PIN);
}

/*
 * @brief Helper for Access BSP EEPROM, chip select is active high
 *
 * @param [in]      output - true/false
 * @param [out]     none
 * @return pin value
 *
 * @details
 *
 * @sa
 */
void setSPI1Cs(bool set)
{
    if(set == true)
    {
        adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_11);
    }
    else
    {
        adi_gpio_SetLow(ADI_GPIO_PORT2, ADI_GPIO_PIN_11);
    }
}

uint32_t BSP_RegisterIRQCallback(ADI_CB const *intCallback, void * hDevice)
{
  uint32_t error = SUCCESS;
  do

  {
    NVIC_DisableIRQ(SYS_GPIO_INTA_IRQn);
     if(ADI_GPIO_SUCCESS != adi_gpio_InputEnable(PHY_IRQ_PORT, PHY_IRQ_PIN, true))
    {
        error = FAILURE;
    }
    /* Enable pin interrupt on group interrupt A */
    if(ADI_GPIO_SUCCESS != (adi_gpio_SetGroupInterruptPins(PHY_IRQ_PORT, ADI_GPIO_INTA_IRQ, PHY_IRQ_PIN )))
    {
        break;
    }
    /* Register the callback */
    if(ADI_GPIO_SUCCESS != (adi_gpio_RegisterCallback (ADI_GPIO_INTA_IRQ, (ADI_CALLBACK)intCallback, (void*)hDevice)))
    {
        break;
    }
    // GPIO priority must be lower then SPI priority!!!
    NVIC_SetPriority(SYS_GPIO_INTA_IRQn , 20);
    NVIC_DisableIRQ(SYS_GPIO_INTA_IRQn);
  }while(0);
  return error;
}

/**
 * GPIO initialization
 *
 * @brief  GPIO initialization
 *
 */
uint32_t GpioInit(void)
{
	uint32_t error = SUCCESS;
	/* init the GPIO service */
	do
	{
		if (ADI_GPIO_SUCCESS != (error = adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE)))
		{
			error = ADI_GPIO_FAILURE;
			break;
		}
        /* LED system, heartbeat*/
        adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_4);
        adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_5);
        if(ADI_GPIO_SUCCESS !=  adi_gpio_OutputEnable(ADI_GPIO_PORT2, ( ADI_GPIO_PIN_4 | ADI_GPIO_PIN_5), true))
        {
            error = FAILURE;
            break;
        }

        /*EEPROM CS p59*/
        if(ADI_GPIO_SUCCESS !=  adi_gpio_OutputEnable(ADI_GPIO_PORT1, ( ADI_GPIO_PIN_10),  true))
        {
            error = FAILURE;
            break;
        }

           adi_gpio_SetLow(ADI_GPIO_PORT1, ADI_GPIO_PIN_10);

      /*Board config pins , pins 19,20,21,22*/
         if(ADI_GPIO_SUCCESS != adi_gpio_InputEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_7
                                                        | ADI_GPIO_PIN_8
                                                        | ADI_GPIO_PIN_9
                                                        | ADI_GPIO_PIN_10 , true))
        {
            error = FAILURE;
        }
        /*pull up*/
        if(ADI_GPIO_SUCCESS !=adi_gpio_PullUpEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_7
                                                        | ADI_GPIO_PIN_8
                                                        | ADI_GPIO_PIN_9
                                                        | ADI_GPIO_PIN_10, true))
        {
            error = FAILURE;
        }
        /* LED pins*/
        if(ADI_GPIO_SUCCESS !=  adi_gpio_OutputEnable(ADI_GPIO_PORT2, ( ADI_GPIO_PIN_3
                                                        | ADI_GPIO_PIN_4
                                                        | ADI_GPIO_PIN_5
                                                        | ADI_GPIO_PIN_6  ), true))
        {
            error = FAILURE;
            break;
        }
        adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_3);
        adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_4);
        adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_5);
        adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_6);

        /*MAC IRQ, pin47*/

        //BSP_RegisterIRQCallback((ADI_CALLBACK const*)pinIntCallback);
#ifdef MDIO_GPIO
        /*MDI to be input, pin 51*/
        if(ADI_GPIO_SUCCESS != adi_gpio_InputEnable(ADI_GPIO_PORT1, ADI_GPIO_PIN_4, true))
        {
            error = FAILURE;
        }

         /*MDC P0_14 pin46, */
        if(ADI_GPIO_SUCCESS != adi_gpio_OutputEnable(ADI_GPIO_PORT1, ADI_GPIO_PIN_2 , true))
        {
            error = FAILURE;
            break;
        }
        BSP_ChangeMDIPinDir(false);//input
#else
        /*SPI1 CS1, pin33*/
        if(ADI_GPIO_SUCCESS != adi_gpio_OutputEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_11 , true))
        {
            error = FAILURE;
            break;
        }
        adi_gpio_SetLow(ADI_GPIO_PORT2, ADI_GPIO_PIN_11); //deselect eeprom
#endif
        /*SPI0 CS2, pin21*/
        if(ADI_GPIO_SUCCESS != adi_gpio_OutputEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_8 , true))
        {
            error = FAILURE;
            break;
        }
        adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_8);

        /*HW ResetADIN1100, pin40*/
        if(ADI_GPIO_SUCCESS != adi_gpio_OutputEnable(ADI_GPIO_PORT0, ADI_GPIO_PIN_8 , true))
        {
            error = FAILURE;
            break;
        }
        adi_gpio_SetHigh(ADI_GPIO_PORT0, ADI_GPIO_PIN_8);

	}while (0);
    return(uint32_t)(error);
}
#ifdef ADIN1110
/**
 * Spi0 peripheral initialization.
 *
 * @brief Initialization of the Spi0 peripheral for clock, mode of operation etc.
 *
 * @param none
 *
 *@return SUCCESS/FAILURE
 */
uint32_t InitSpi0(void)
{
  uint32_t error = SUCCESS;
  do
  {
    /* Initialize SPI */
    if(adi_spi_Open(SPI_DEVICE_0,spidevicememSPI0, ADI_SPI_MEMORY_SIZE, &hSpi0) != ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }

    /* throttle bitrate to something the controller can reach */
    if(adi_spi_SetBitrate(hSpi0, SPI_CLK_DEVICE_0) != ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }
    /* Set IRQMODE. In this case we are setting it to the default value  */
    /* This code sequence is just calling out the fact that this API would be required  */
    /* for short bursts (less than the size of the FIFO) in PIO (interrupt) mode        */
    if(adi_spi_SetIrqmode(hSpi0, 1u)!= ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }

    /* Set the chip select */
    if(adi_spi_SetChipSelect(hSpi0, ADI_SPI_CS0)!=ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }

    /* Set the SPI clock phase */
#if defined(ADIN_S1)
    if(adi_spi_SetClockPhase(hSpi0, true) != ADI_SPI_SUCCESS)
#else
    if(adi_spi_SetClockPhase(hSpi0, false) != ADI_SPI_SUCCESS)
#endif
    {
      return FAILURE;
    }

    /* Set master mode */
    if(adi_spi_SetMasterMode(hSpi0, true) != ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }

    if(adi_spi_SetContinuousMode(hSpi0, true) != ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }

  }while(0);

  return (error);
}

/**
 * Spi0 read/write.
 *
 * @brief Perform Spi0 write / read operations.
 * @note: Implement a non blobking call
 *
 * @param pBufferTx pointer to transmit buffer
 * @param pBufferRx pointer to receive buffer
 * @param useDma bollean flag for DMA mode
 * @param nbBytes number of bytes to write / read
 *
 *@return SUCCESS/FAILURE
 */
uint32_t BSP_spi0_write_and_read(uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes, bool useDma)
{
    uint32_t error = 0;
    /* SPI transceiver instance */
    ADI_SPI_TRANSCEIVER Transceiver;
    do
    {
        if (nbBytes > SPI_BUFFERSIZE )
        {
            error = FAILURE;//ADI_ETH_HAL_INIT_ERROR;
            break;
        }

        /* Initialize the transceiver */
        Transceiver.pReceiver           =   pBufferRx;
        Transceiver.ReceiverBytes       =   nbBytes;
        Transceiver.nRxIncrement        =   1;
        Transceiver.pTransmitter        =   pBufferTx;
        Transceiver.TransmitterBytes    =   nbBytes;
        Transceiver.nTxIncrement        =   1;
        Transceiver.bDMA = useDma;
        Transceiver.bRD_CTL = false;

        /* Transmit and receive */
        if (adi_spi_MasterSubmitBuffer(hSpi0, &Transceiver) != ADI_SPI_SUCCESS)
        {
            error = FAILURE;//ADI_ETH_HAL_INIT_ERROR;
            break;
        }
    }while(0);
    return (error);
}

/**
 * Spi0 callback.
 *
 * @brief Register Spi0 callback.
 *
 * @param pfCallback     Callback function.
 * @param pCBParam      Parameters for pfCallback
 *
 *@return SUCCESS/FAILURE
 */
uint32_t BSP_spi0_register_callback(ADI_CALLBACK const *pfCallback, void *const pCBParam)
{
    ADI_SPI_RESULT result;

    result = adi_spi_RegisterCallback (hSpi0, ( ADI_CALLBACK) pfCallback, pCBParam);

    return result;
}
#endif

/*Onboard EEPROM*/
#ifdef SPI_EEPROM
uint32_t SpiInitFlash(void)
{
  uint32_t error = SUCCESS;
  do
  {
    /* Initialize SPI */
    if(adi_spi_Open(SPI_DEVICE_1,spidevicememFLASH, ADI_SPI_MEMORY_SIZE, &hSpiFlash) != ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }

    /* throttle bitrate to something the controller can reach */
    if(adi_spi_SetBitrate(hSpiFlash, SPI_CLK_FLASH) != ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }
    /* Set IRQMODE. In this case we are setting it to the default value  */
    /* This code sequence is just calling out the fact that this API would be required  */
    /* for short bursts (less than the size of the FIFO) in PIO (interrupt) mode        */

    if(adi_spi_SetIrqmode(hSpiFlash, 1u)!= ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }

    /* set the chip select */
    if(adi_spi_SetChipSelect(hSpiFlash, ADI_SPI_CS3)!=ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }
    /* Set the SPI clock phase */



    if(adi_spi_SetClockPolarity(hSpiFlash, false) != ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }


    if(adi_spi_SetClockPhase(hSpiFlash, false) != ADI_SPI_SUCCESS)
    {
      return FAILURE;
    }

    /* Set master mode */
    if(adi_spi_SetMasterMode(hSpiFlash, true) != ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }

    if(adi_spi_SetContinuousMode(hSpiFlash, true) != ADI_SPI_SUCCESS)
    {
      error = FAILURE;
      break;
    }

  }while(0);

  return (error);
}

/**
 * Spi0 read/write.
 *
 * @brief Perform Spi0 write / read operations.
 * @note: Implement a blobking call
 *
 * @param pBufferTx pointer to transmit buffer
 * @param pBufferRx pointer to receive buffer
 * @param useDma bollean flag for DMA mode
 * @param nbBytes number of bytes to write / read
 *
 *@return SUCCESS/FAILURE
 */
uint32_t BSP_spi1_write_and_read(uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes)
{
    uint32_t error = 0;
    /* SPI transceiver instance */
    ADI_SPI_TRANSCEIVER Transceiver;
    do
    {
        if (nbBytes > SPI_BUFFERSIZE )
        {
            error = FAILURE;
            break;
        }
        /* set the chip select */


        setSPI1Cs(true);
        /* Initialize the transceiver */
        Transceiver.pReceiver           =   pBufferRx;
        Transceiver.ReceiverBytes       =   nbBytes;
        Transceiver.nRxIncrement        =   1;
        Transceiver.pTransmitter        =   pBufferTx;
        Transceiver.TransmitterBytes    =   nbBytes;
        Transceiver.nTxIncrement        =   1;
        Transceiver.bDMA = true;
        Transceiver.bRD_CTL = false;

        /* Transmit and receive */
        if (adi_spi_MasterReadWrite(hSpiFlash, &Transceiver) != ADI_SPI_SUCCESS)
        {
            error = FAILURE;
            break;
        }
        setSPI1Cs(false);
    }while(0);
    return (error);
}
#endif

#if defined(ADIN1100)
/*
 * @brief Initialisation for SPI for MDIO emulation
 *
 * @param [in]      none
 * @param [out]     none
 * @return
 *
 * @details
 *
 * @sa
 */
uint32_t InitSpi2(void)
{
    uint32_t error = SUCCESS;
    do
    {
       /* Initialize SPI */
        if(adi_spi_Open(SPI_DEVICE_2,spidevicememSPI2, ADI_SPI_MEMORY_SIZE, &hSpi2) != ADI_SPI_SUCCESS)
        {
            error = FAILURE;
            break;
        }

        /* throttle bitrate to something the controller can reach */
        if(adi_spi_SetBitrate(hSpi2, SPI_CLK_MDIO) != ADI_SPI_SUCCESS)
        {
            error = FAILURE;
            break;
        }
        /* Set IRQMODE. In this case we are setting it to the default value  */
        /* This code sequence is just calling out the fact that this API would be required  */
        /* for short bursts (less than the size of the FIFO) in PIO (interrupt) mode        */
        if(adi_spi_SetIrqmode(hSpi2, 1u)!= ADI_SPI_SUCCESS)
        {
            error = FAILURE;
            break;
        }

        /* set the chip select */
        if(adi_spi_SetChipSelect(hSpi2, ADI_SPI_CS0)!=ADI_SPI_SUCCESS)
        {
            error = FAILURE;
            break;
        }
/*
        if(adi_spi_SetClockPolarity(hSpi2, true) != ADI_SPI_SUCCESS)
        {
            error = FAILURE;
            break;
       }
*/
        /* Set the SPI clock phase */
/*
        if(adi_spi_SetClockPhase(hSpi2, false) != ADI_SPI_SUCCESS)
        {
          return FAILURE;
        }
*/
        /* Set master mode */
        if(adi_spi_SetMasterMode(hSpi2, true) != ADI_SPI_SUCCESS)
        {
            error = FAILURE;
            break;
        }

        if(adi_spi_SetContinuousMode(hSpi2, true) != ADI_SPI_SUCCESS)
        {
            error = FAILURE;
            break;
        }
#if 0
        if(adi_spi_SetWiredORMode(hSpi2, true) != ADI_SPI_SUCCESS)
        {
            error = FAILURE;
            break;
        }
#endif
        *pREG_SPI2_CTL |= 0x00000010;//adi_spi_SetWiredORMode

     }while(0);

    return (error);
}

/*
 * @brief SPI Read/Write for  MDIO Emulation
 *
 * @param [in]      pBufferTx - pointer to data to transmit
 * @param [in]      nbBytes - number of bytes to transmit/receive
 * @param [out]     pBufferRx - pointer to data to receive
 * @return pin value
 *
 * @details
 *
 * @sa
 */
uint32_t BSP_spi2_write_and_read(uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes)
{
    uint32_t error = 0;
    /* SPI transceiver instance */
    ADI_SPI_TRANSCEIVER Transceiver;
    do
    {
        if (nbBytes > SPI_BUFFERSIZE )
        {
            error = FAILURE;
            break;
        }

        /* Initialize the transceiver */
        Transceiver.pReceiver           =   pBufferRx;
        Transceiver.ReceiverBytes       =   nbBytes;
        Transceiver.nRxIncrement        =   1;
        Transceiver.pTransmitter        =   pBufferTx;
        Transceiver.TransmitterBytes    =   nbBytes;
        Transceiver.nTxIncrement        =   1;
        Transceiver.bDMA = true;
        Transceiver.bRD_CTL = false;

        /* Transmit and receive */
        if (adi_spi_MasterReadWrite(hSpi2, &Transceiver) != ADI_SPI_SUCCESS)
        {
            error = FAILURE;
            break;
        }
    }while(0);
    return (error);
}
#endif
/*
 * @brief Change speed of SPI  for  MDIO Emulation
 *
 * @param [in]      speed - Clock frequency in Hz
 * @param [out]     none
 * @return none
 *
 * @details
 *
 * @sa
 */
void changeMDIOSpeed(uint32_t speed)
{
//    adi_spi_SetBitrate(hSpi2, speed);
}


uint32_t BSP_SpiInit(void)
{
#ifdef ADIN1110
    InitSpi0();
#endif

/*Onboard EEPROM*/
#ifdef SPI_EEPROM
    InitSpi1();
#endif

#ifdef ADIN1100
#ifndef MDIO_GPIO
    InitSpi2();
#endif
#endif
    return 0;
}


#define RESET_RX_CNTR_LIMIT	400


volatile static uint32_t gNumGp0Timeouts = 0u;
static uint64_t volatile TimeHalfSeconds = 0;
static uint32_t volatile LastTimeLow = 0;




void BSP_SystemTimeCheck(void)
{
    NVIC_ClearPendingIRQ(TMR0_EVT_IRQn);
    uint16_t currentVal = 0;

    adi_tmr_GetCurrentCount(ADI_TMR_DEVICE_GP0, &currentVal);
    if(LastTimeLow < currentVal)
    {
        TimeHalfSeconds += ADUCM4050_TIMER_SPEED_10HZ;
        __ASM volatile ("dsb");
    }
    LastTimeLow = currentVal;
}

uint64_t BSP_SystemTimeCurrentTime64(void)
{
   uint64_t ret;

   uint16_t currentVal = 0;
   uint32_t subSecondOffset = 0;
   NVIC_ClearPendingIRQ(TMR0_EVT_IRQn);
   adi_tmr_GetCurrentCount(ADI_TMR_DEVICE_GP0, &currentVal);
   if(LastTimeLow < currentVal)
   {
      TimeHalfSeconds += ADUCM4050_TIMER_SPEED_10HZ;
      __ASM volatile ("dsb");
   }
   LastTimeLow = currentVal;
   ret = TimeHalfSeconds;

   if(currentVal <= ADUCM4050_TIMER_SPEED_10HZ)
   {
        subSecondOffset = ADUCM4050_TIMER_SPEED_10HZ - currentVal;
   }
   ret += subSecondOffset;

   return ret;
}

uint32_t BSP_SysNow(void)
{
   uint64_t time = BSP_SystemTimeCurrentTime64();
   return (uint32_t) (time / (ADUCM4050_TIMER_SPEED_10HZ / 100));
}


void GP0CallbackFunction(void *pCBParam, uint32_t Event, void  * pArg)
{

  NVIC_ClearPendingIRQ(TMR0_EVT_IRQn);
    /* IF(Interrupt occurred because of a timeout) */
    if ((Event & ADI_TMR_EVENT_TIMEOUT) == ADI_TMR_EVENT_TIMEOUT)
    {
       BSP_SystemTimeCheck();
       __ASM volatile ("dsb");
    }
}
static uint32_t timerEnable(bool enable)
{
    uint32_t error = 0;

    if( adi_tmr_Enable(ADI_TMR_DEVICE_GP0, enable) != ADI_TMR_SUCCESS)
    {
        error = 1;
    }
 return error;
}

static uint32_t timerInit(void)
{
  ADI_TMR_CONFIG        tmrConfig;
  uint32_t error = 0;
  do
  {
    timerEnable(false);
    /* Set up GP0 callback function */
    if( adi_tmr_Init(ADI_TMR_DEVICE_GP0, GP0CallbackFunction, NULL, true) != ADI_TMR_SUCCESS)
    {
      error = 1;
      break;
    }


    tmrConfig.bCountingUp  = false;
    tmrConfig.bPeriodic    = true;
    tmrConfig.ePrescaler   = ADI_TMR_PRESCALER_64;
    tmrConfig.eClockSource = ADI_TMR_CLOCK_HFOSC;
    tmrConfig.nLoad        = ADUCM4050_TIMER_SPEED_10HZ;
    tmrConfig.nAsyncLoad   = ADUCM4050_TIMER_SPEED_10HZ;
    tmrConfig.bReloading   = false;
    tmrConfig.bSyncBypass  = false;

     if( adi_tmr_ConfigTimer(ADI_TMR_DEVICE_GP0, &tmrConfig) != ADI_TMR_SUCCESS)
      {
        error = 1;
        break;
      }

  }while(0);

    return error;
}

/**
 * UART initialization
 *
 * @brief  UART initialization
 *
 */
uint32_t UartInit(void)
{
    uint32_t error = SUCCESS;
    do
    {
        /* Open the bidirectional UART device. */
        if( ADI_UART_SUCCESS != adi_uart_Open(UART_DEVICE_NUM, ADI_UART_DIR_BIDIRECTION, UartDeviceMem, ADI_UART_MEMORY_SIZE, &hDeviceUart))
        {
            error = FAILURE;
            break;
        }

        /* Configure  UART device with NO-PARITY, ONE STOP BIT and 8bit word length. */
        if ((error = adi_uart_SetConfiguration(hDeviceUart,
                                        ADI_UART_NO_PARITY,
                                        ADI_UART_ONE_AND_HALF_TWO_STOPBITS,
                                        ADI_UART_WORDLEN_8BITS)) != ADI_UART_SUCCESS)
        {
            error = FAILURE;
            break;
        }

        /* Baud rate div values are calcuated for PCLK 26Mhz. Please use the
        host utility UartDivCalculator.exe provided with the installer"
        */
        adi_uart_ConfigBaudRate(hDeviceUart, 3, 2, 719, 3);//115k
        /* Register a callback. */
#if 0
        if(adi_uart_RegisterCallback(hDeviceUart, UARTCallback, NULL) != ADI_UART_SUCCESS)
        {
            error = FAILURE;
            break;
        }
#endif
        if(adi_uart_EnableFifo(hDeviceUart,true) != ADI_UART_SUCCESS)
        {
            error = FAILURE;
            break;
        }
      }while(0);

    return (error);
}

uint32_t sumbmitRxBuffer(uint8_t *pBuffer, int nbBytes)
{
    uint32_t error = 0;
    adi_uart_FlushRxChannel(hDeviceUart);
    error = adi_uart_SubmitRxBuffer(hDeviceUart, pBuffer, nbBytes, DMA_OFF);
return (error);
}

uint32_t sumbmitTxBuffer(uint8_t *pBuffer, int nbBytes)
{
    uint32_t error = 0;
   // 		while (!getUartTxEmpty());
    adi_uart_Write(hDeviceUart, pBuffer, nbBytes,DMA_ON, &error );
return (error);
}



/**
 * MCU system initialization
 *
 * @brief  MCU system initialization
 *
 */
static uint32_t ClockInit(void)
{
    uint32_t error = SUCCESS;

    /* Power initialization */
    if(ADI_PWR_SUCCESS != adi_pwr_Init())
    {
        error = FAILURE;
    }
    if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_HCLK,1))
    {
        error = FAILURE;
    }
    if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_PCLK,1))
    {
        error = FAILURE;
    }

    return (error);
}

/**
 * Disable Watchdog
 *
 * @brief  Watchdog disable
 *
 */
static uint32_t WdtDisable(void)
{
   /*Stop watch dog timer(ADuCM3029)*/
   pADI_WDT0->CTL = 0xC9;
   return SUCCESS;/* ToDo: return proper error code */
}

/**
 * System Initialization
 *
 * @brief  System Initialization
 *
 */
uint32_t BSP_InitSystem(void)
{
    uint32_t error = SUCCESS;

    do
    {
        SystemInit();
        if((error = ClockInit()) != SUCCESS)
        {
            break;
        }

        if((error = WdtDisable()) != SUCCESS)
        {
            break;
        }

        if((error = adi_initpinmux()) != SUCCESS)
        {
            break;
        }

        if((error = GpioInit())!= SUCCESS)
        {
            break;
        }

        if((error = BSP_SpiInit()) != SUCCESS)
        {
            break;
        }
        if((error = UartInit()) != SUCCESS)
        {
           break;
        }
        if((error = timerInit()) != SUCCESS)
        {
            break;
        }

        if((error = timerEnable(true)) != SUCCESS)
        {
            break;
        }

    }while(0);

    return (error);
}


/*
* Debug outout to UART @115200 kbps
*/
void msgWrite(char * ptr)
{
   int len = strlen(ptr);
    /*Actual message*/
    sumbmitTxBuffer((uint8_t*)ptr,len);
}

void common_Fail(char *FailureReason)
{
    char fail[] = "Failed: ";
    char term[] = "\n\r";

    /* Ignore return codes since there's nothing we can do if it fails */
    msgWrite(fail);
    msgWrite(FailureReason);
    msgWrite(term);
 }

void common_Perf(char *InfoString)
{
    char term[] = "\n\r";

    /* Ignore return codes since there's nothing we can do if it fails */
    msgWrite(InfoString);
    msgWrite(term);
}

