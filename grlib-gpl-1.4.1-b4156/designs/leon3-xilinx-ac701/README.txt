This leon3 design is tailored to the Xilinx Artix-7 AC701 board
---------------------------------------------------------------

NOTE: The SGMII Ethernet interface does currently NOT work on this
design.

http://www.xilinx.com/ac701

Note #1: After successfully programmed the FPGA using the target 
'ise-prog-fpga' user might have to press the 'CPU RESET' button 
in order to successfully complete the calibration process in the MIG.

Note #2: This design requires that the GRLIB_SIMULATOR variable is
correctly set. Please refer to the documentation in doc/grlib.pdf for
additional information.

Note #3: The Vivado flow and parts of this design are still
experimental. Currently the design configuration should be left as-is.

Note #4: You must have Vivado 2014.4.1 in your path for the make targets to work.

The XILINX_VIVADO variable must be exported for the mig_7series target
to work correctly: export XILINX_VIVADO


Simulation and synthesis
------------------------

The design uses the Xilinx MIG memory interface with an AHB-2.0
interface. The MIG source code cannot be distributed due to the
prohibitive Xilinx license, so the MIG must be re-generated with 
Vivado before simulation and synthesis can be done.

Xilinx MIG interface will automatically be generated when 
Vivado is launched  

To simulate using XSIM and run systest.c on the Leon design using the memory 
controller from Xilinx use the make targets:

  make vivado-launch

To simulate using Modelsim and run systest.c on the Leon design using 
the memory controller from Xilinx use the make targets:

  make map_xilinx_7series_lib sim
  make mig_7series
  make sim-launch

To simulate using the Aldec Riviera WS flow use the following make targets:

  make riviera_ws               # creates riviera workspace
  make map_xilinx_7series_lib   # compiles and maps xilinx sim libs
  make mig_7series              # generates MIG IP and adds to riviera project
  make sgmii_7series            # same for SGMII adapter
  make riviera                  # compile full project
  make riviera-launch           # launch simulation

To synthesize the design, do

  make vivado

and then use iMPACT programming tool:
  
  make ise-prog-fpga

to program the FPGA.

After successfully programmed the FPGA using the target 'ise-prog-fpga' user might 
have to press the 'CPU RESET' button in order to successfully complete the calibration
process in the MIG. Led 1 and led 2 should be constant green if the Calibration 
process has been successful.

If user tries to connect to the board and the MIG has not been calibrated successfully
'grmon' will output:

$ grmon -xilusb -u -nb
  
  GRMON2 LEON debug monitor v2.0.43-2-g95d293c internal version
  
  Copyright (C) 2013 Aeroflex Gaisler - All rights reserved.
  For latest updates, go to http://www.gaisler.com/
  Comments or bug-reports to support@gaisler.com
  
Parsing -xilusb
Parsing -u
Parsing -nb

Commands missing help:
 datacache

Xilusb: Cable type/rev : 0x3
 JTAG chain (1):
xc7k325t

AMBA plug&play not found!
Failed to initialize target!
Exiting GRMON

The MIG and SGMII IP can be disabled either by deselecting the memory
controller and Gaisler Ethernet interface in 'xconfig' or manually
editing the config.vhd file.  When no MIG and no SGMII block is
present in the system normal GRLIB flow can be used and no extra
compile steps are needed. Also when when no MIG is present it is
possible to control and set the system frequency via xconfig.  Note
that the system frequency can be modified via Vivado when the MIG is
present by modifying within specified limits for the MIG IP.

Compiling and launching modelsim when no memory controller and no
ethernet interface is present using Modelsim/Aldec simulator:

  make vsim
  make vsim-launch

Simulation options
------------------

All options are set either by editing the testbench or specify/modify the generic 
default value when launching the simulator. For Modelsim use the option "-g" i.e.
to enable processor disassembly to console launch modelsim with the option: "-gdisas=1"

USE_MIG_INTERFACE_MODEL - Use MIG simulation model for faster simulation run time
(Option can now be controlled via 'make xconfig')

disas - Enable processor disassembly to console

System Clock (when using Xilinx MIG)
------------------------------------

The system clock is derived from the Xilinx MIG IP when the MIG IP is used. 

To modify the system clock when the MIG IP is used the user should adjust 
the parameter TimePeriod in the project file used for generating the MIG IP 
and regenerate the MIG IP. The parameter used to generate the MIG is located in the file 
.../grlib/boards/xilinx-ac701-xc7a200t/mig.prj. 

The AHB system clock period is The PHY clock period set in the project file divided by 4.

Parameters used by Xilinx IP generator to set and generate clocks:

 * PHY Clock Period. Choose the clock period for the external memory frequency 
   by adjusting the line:

       <TimePeriod>2500</TimePeriod>

 * PHY to system clock ratio. The PHY operates at the frequency set by 
   'TimePeriod' and the controller operates only at 1/4. (PHYRatio 2:1 
   will not work due to no support in GRLIB MIG interface.)

        <PHYRatio>4:1</PHYRatio>

 * Board input frequency. Select the period for the MIG PLL input clock. 
   Allowable input clock frequencies are listed in the MIG IP documentation 
   available at Xilinx.com.  

        <InputClkFreq>200</InputClkFreq>

Debug UART
----------

The normal UART and DEBUG UART share pins in the design.

The normal UART and DEBUG UART share pins in the design.

UART mode is selected by GPIO DIP SWITCH 3:
 0 - Normal UART
 1 - Debug UART


SDIO CARD
---------

Communication with an SD card can be done in one of two modes: the SD mode 
or the SPI mode. By default, the SD card operates in the SD mode. The AC701
Ref design only works in SPI mode.

To communicate with the SD card, your program has to place the SD card into 
the SPI mode. To do this, set the MOSI and CS lines to logic value 1 and 
toggle SD CLK for at least 74 cycles. After the 74 cycles (or more) have occurred, 
your program should set the CS line to 0 and send the command CMD0:
01 000000 00000000 00000000 00000000 00000000 1001010 1
This is the reset command, which puts the SD card into the SPI mode if executed 
when the CS line is low. The SD card will respond to the reset command by sending 
a basic 8-bit response on the MISO line. 

 The first bit is always a 0, while the other bits specify:
 
  #7 - Always '0'
  #6 - Parameter error
  #5 - Address error
  #4 - Erase seq error
  #3 - CRC error
  #2 - Illegal reset
  #1 - Erase reset
  #0 - Idle state

If the command you sent was successfully received, then you will receive 
the message b00000001.

To receive this message, your program should continuously toggle the SD CLK 
signal and observe the MISO line for data, while keeping the MOSI line high 
and the CS line low. Your program can detect the message, because every message 
begins with a 0 bit, and when the SD card sends no data it keeps the MISO 
line high. Note that the response to each command is sent by the card a few SD CLK 
cycles later. If the expected response is not received within 16 clock cycles after 
sending the reset command, the reset command has to be sent again

Design specifics
----------------

* The DDR3 controller is implemented with Xilinx MIG 7-Series 2.0 and 
  runs of the 200 MHz clock. The DDR3 memory runs at 400 MHz
  (DDR3-800). grmon-2.0.30-74 or later is needed to detect the
   DDR3 memory.

* The AHB clock is generated by the MMCM module in the DDR3
  controller, and can be controlled via Vivado. When the 
  MIG DDR3 controller isn't present the AHB clock is generated
  from CLKGEN, and can be controlled via xconfig

* System reset is mapped to the CPU RESET button

* DSU break is mapped to GPIO east button

* LED 0 indicates processor in debug mode

* LED 1 indicates processor in error mode, execution halted

* LED 2 indicates DDR3 PHY initialization done (Only valid when MIG is present)

* LED 3 indicates internal PLL has locked (Only valid when MIG isn't present)

* SPI flash prom can be read at address 0. It can be programmed
  with GRMON version 2.0.56 or later.

* The application UART1 is connected to the USB/RS232 connector

* The JTAG DSU interface is enabled and accesible via the JTAG port.
  Start grmon with -xilusb to connect.

* Output from GRMON is:

  grmon -xilusb -u -nb

  GRMON2 LEON debug monitor v2.0.46-12-g452afa7 internal version
  
  Copyright (C) 2013 Aeroflex Gaisler - All rights reserved.
  For latest updates, go to http://www.gaisler.com/
  Comments or bug-reports to support@gaisler.com
  
  This internal version will expire on 28/01/2015

Parsing -xilusb
Parsing -u
Parsing -nb

Commands missing help:
 datacache

Xilusb: Cable type/rev : 0x3 
 JTAG chain (1): xc7a200t 
  GRLIB build version: 4141
  Detected frequency:  100 MHz
  
  Component                            Vendor
  LEON3 SPARC V8 Processor             Aeroflex Gaisler
  AHB Debug UART                       Aeroflex Gaisler
  JTAG Debug Link                      Aeroflex Gaisler
  GR Ethernet MAC                      Aeroflex Gaisler
  AHB/APB Bridge                       Aeroflex Gaisler
  LEON3 Debug Support Unit             Aeroflex Gaisler
  Single-port AHB SRAM module          Aeroflex Gaisler
  Xilinx MIG DDR3 Controller           Aeroflex Gaisler
  Single-port AHB SRAM module          Aeroflex Gaisler
  Generic UART                         Aeroflex Gaisler
  Multi-processor Interrupt Ctrl.      Aeroflex Gaisler
  Modular Timer Unit                   Aeroflex Gaisler
  AMBA Wrapper for OC I2C-master       Aeroflex Gaisler
  General Purpose I/O port             Aeroflex Gaisler
  Gaisler RGMII Interface              Aeroflex Gaisler
  
  Use command 'info sys' to print a detailed report of attached cores

grmon2>

* grmon output using Ethernet

grmon -eth -ip 192.168.0.51 -u -nb

 GRMON2 LEON debug monitor v2.0.55-410-g28805c6 internal version
  
  Copyright (C) 2014 Aeroflex Gaisler - All rights reserved.
  For latest updates, go to http://www.gaisler.com/
  Comments or bug-reports to support@gaisler.com
  
  This internal version will expire on 01/08/2015

Parsing -eth
Parsing -ip 192.168.0.51
Parsing -u
Parsing -nb

Commands missing help:
 datacache

 Ethernet startup...
  GRLIB build version: 4148
  Detected frequency:  100 MHz
  
  Component                            Vendor
  LEON3 SPARC V8 Processor             Aeroflex Gaisler
  JTAG Debug Link                      Aeroflex Gaisler
  GR Ethernet MAC                      Aeroflex Gaisler
  AHB/APB Bridge                       Aeroflex Gaisler
  LEON3 Debug Support Unit             Aeroflex Gaisler
  Single-port AHB SRAM module          Aeroflex Gaisler
  Xilinx MIG DDR3 Controller           Aeroflex Gaisler
  Single-port AHB SRAM module          Aeroflex Gaisler
  Generic AHB ROM                      Aeroflex Gaisler
  Generic UART                         Aeroflex Gaisler
  Multi-processor Interrupt Ctrl.      Aeroflex Gaisler
  Modular Timer Unit                   Aeroflex Gaisler
  AMBA Wrapper for OC I2C-master       Aeroflex Gaisler
  General Purpose I/O port             Aeroflex Gaisler
  Gaisler RGMII Interface              Aeroflex Gaisler
  
  Use command 'info sys' to print a detailed report of attached cores

grmon2> load /usr/local32/apps/bench/leon3/dhry.leon3
  40000000 .text                     54.7kB /  54.7kB   [===============>] 100%
  4000DAF0 .data                      2.7kB /   2.7kB   [===============>] 100%
  Total size: 57.44kB (23.53Mbit/s)
  Entry point 0x40000000
  Image /usr/local32/apps/bench/leon3/dhry.leon3 loaded
  
grmon2> verify /usr/local32/apps/bench/leon3/dhry.leon3
  40000000 .text                     54.7kB /  54.7kB   [===============>] 100%
  4000DAF0 .data                      2.7kB /   2.7kB   [===============>] 100%
  Total size: 57.44kB (24.77Mbit/s)
  Entry point 0x40000000
  Image of /usr/local32/apps/bench/leon3/dhry.leon3 verified without errors
  
grmon2> run
Execution starts, 1000000 runs through Dhrystone
Total execution time:                          4.6 s
Microseconds for one run through Dhrystone:    4.6 
Dhrystones per Second:                      218594.8 

Dhrystones MIPS      :                       124.4 

  
  Program exited normally.
  
grmon2>  
