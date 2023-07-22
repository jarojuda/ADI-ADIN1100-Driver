
This example shows basic usage of the frame generator and frame checker featured in the ADIN1100 PHY device.
It also exercises the PCS hardware loopback in the device.

Hardware Setup
==============

The hardware setup requires one EVAL-ADIN1100EBZ board.

The MCU on board sends statuses and results to the host using the UART interface.

LED functionality:
- LED: ON when link is established, blinking while the frame generator is actively sending frames

Software Setup
==============

There is no additional software setup required to run the example.

FRAME_COUNT can be used to change the number of frames generated, by default set to 20000.

For UART output, configure the PC-based serial terminal software for a baudrate of 115200, 8N1.

Operation
=========

1. Build and download the example code to the on board MCU
2. Run the application
3. Observe successful completion of the example code with reported results:

        Frame Generator/Checker Results:
            (FG) Sent:           20000 frames
            (FC) Received:       20000 frames
                 Rx Errors             = 0
                   LEN_ERR_CNT         = 0
                   ALGN_ERR_CNT        = 0
                   SYMB_ERR_CNT        = 0
                   OSZ_CNT             = 0
                   USZ_CNT             = 0
                   ODD_CNT             = 0
                   ODD_PRE_CNT         = 0
                   FALSE_CARRIER_CNT   = 0

