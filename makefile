MCU=atmega328p
F_CPU=16000000
CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-std=c99 -Wall -g -Os -mmcu=${MCU} -DF_CPU=${F_CPU} -I.

ARDUINO_HEADER=/usr/share/arduino/hardware/arduino/cores/arduino/Arduino.h
TARGET=build/main
SRCS=main/main.ino

all: compose build

compose :
	cat ${ARDUINO_HEADER} ${SRCS} >> ${TARGET}.temp
build:
	        ${CC} ${CFLAGS} -o ${TARGET}.bin ${TARGET}.temp
		        ${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex

upload:
	        avrdude -p ${MCU} -c usbasp -U flash:w:${TARGET}.hex:i -F -P usb

clean:
	        rm -f *.bin *.hex *.temp
		
