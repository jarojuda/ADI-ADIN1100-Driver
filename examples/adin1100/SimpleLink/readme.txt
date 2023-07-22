
This example shows how to initialize and configure the PHY, bring up the link and read link status. It also shows how to use
a callback function to get notifications when link status changes.

Hardware Setup
==============

The hardware setup requires two EVAL-ADIN1100EBZ boards, connected via cable.

The MCU on each board will run the same code and send information related to code execution to the host PC via the UART interface.
On one board (Board1) remove the reset linker on P8 and on the other board (Board2) set the reset linker in P8-PHY RESET position.
The UART interface on Board1 should be connected to the host PC, the Board2 UART is optional.

LED functionality:
- DS1: ON when link is established (set by hardware)
- uC1: ON when link is established (set by software)
- uC2: heartbeat pulse indicating software activity

Software Setup
==============

There is no additional software setup required to run the example.

For UART output, configure the PC-based serial terminal software for a baudrate of 115200, 8N1.

Operation
=========

1. Build and download the example code to the MCU on Board1
2. Run the application on Board1
3. Build and download the example code to the MCU on Board2
4. Run the application on Board2
5. The UART messages received from Board1 (and optionally Board2) will show the PHY is initialized and configured, for example:

    Found PHY device at address 1
      - Supports 2.4V Tx level
      - Supports PMA local loopback
    Link Up

6. LEDs indicate the link status and software activity:
    - DS1 and uC2 LEDs becomes ON on both boards, showing the link has been established
    - uC2 LED pulses indicating software activity

7. Press the reset button on Board2 to keep the MCU and ADIN1100 in reset
    - On Board2, all LEDs are OFF
    - On Board1, DS1 and uC2 are OFF indicates there is no link. A "Link Down" message is printed to UART

8. Depress the reset button on Board2 to release the MCU and ADIN1100 from reset
    - On Board2, DS1 and uC2 become ON indicating the link is established, uC1 pulses showing software activity
    - On Board1, DS1 and uC2 become ON indicating the link is established. A "Link Up" message is printed to UART

9. Press the reset button on Board1 to keep the ADIN1100 in reset (code execution on the MCU continues)
    - On Board2, DS1 and uC2 LEDs are OFF, indicating there is no link. uC1 continues to pulse indicating sofwtare activity
    - On Board1, DS1 is OFF indicates there is no link. uC2 remains ON as the software cannot read the status from the ADIN1100
      to determine the link has been brought down. An "MDIO communication error" message is printed to the UART

8. Depress the reset button on Board1 to bring the link up again
    - On Board2, DS1 and uC2 become ON indicating the link is established
    - On Board1, DS1 becomes ON indicating the link is established. A "Link Up" message is printed to UART
