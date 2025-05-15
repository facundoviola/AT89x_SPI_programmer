# AT89x_SPI_programmer
AT89x SPI programmer / SPI Interface using RS-235 signals

## Introduction
This software allows to program the family of AT89x micro-controllers (only the devices with support for In-System Programmable (ISP) Flash Memory through the SPI protocol.

The SPI part also could be used independently to create a SPI master node (without CS signal).

## Brief description 
* readhex.c: Read an HEX formatted file and store its content in a nested list.
* spi.c: By means of the signals (RTS, CTS, DTR) in a RS-232 port implement the SPI protocol.