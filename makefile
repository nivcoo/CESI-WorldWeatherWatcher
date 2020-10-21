# Makefile to compile an upload sketch to an arduino board
#
# Requirements :
# - Make sure PLATFORM and BOARD correspond to your hardware
# - arduino-cli with your board installed
#
#  If you meet the requirements just run 'make'
#  otherwise you should run 'make requirements' first
#  (you need to be sudo to install requirements

# Parameters
PLATFORM = arduino:avr
BOARD = $(PLATFORM):mega
PORT = /dev/ttyACM0 
BUILD_DIR = $(shell pwd)/build
RANDOM = $(shell bash -c 'echo $$RANDOM')

all: build upload


requirements:
ifneq "$(shell arduino-cli version | cut -c1-11)" "arduino-cli" 
	curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sudo BINDIR=/usr/bin sh
endif
	arduino-cli core install $(PLATFORM)

build:
	sed -i 's#batchNumber = "\w*"#batchNumber = "C3W20_$(RANDOM)"#' main/main.ino
	mkdir -p $(BUILD_DIR)
	arduino-cli compile --fqbn $(BOARD) main/main.ino --build-path $(BUILD_DIR)

upload:
	arduino-cli upload -t -p $(PORT) --fqbn $(BOARD) --input-dir $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) main/build

