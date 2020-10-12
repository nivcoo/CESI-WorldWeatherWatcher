# Makefile to compile an upload sketch to an arduino board
#
# Requirements :
# - Make sure PLATFORM and BOARD correspond to your hardware
# - arduino-cli with your board installed
#
#  If you meet the requirements just run 'make' and then 'make upload'
#  otherwise you should run 'make requirements' first
#

# Parameters
PLATFORM = arduino:avr
BOARD = $(PLATFORM):uno
PORT = /dev/ttyACM0 
BUILD_DIR = $(shell pwd)/build

all: build


requirements:
	curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/local/bin sh
	arduino-cli core install $(PLATFORM)

build:
	mkdir -p $(BUILD_DIR)
	arduino-cli compile --fqbn $(BOARD) main/main.ino --build-path $(BUILD_DIR)

upload:
	arduino-cli upload -t -p $(PORT) --fqbn $(BOARD) --input-dir $(BUILD_DIR)

clean:
	rm -rf build
