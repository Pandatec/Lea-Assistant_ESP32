# only for unix cli use

# replace with your port
PORT = /dev/tty.usbserial-14440

build:
	idf.py build

flash:
	idf.py -p $(PORT) flash monitor

monitor:
	idf.py -p $(PORT) monitor

.PHONY: build flash
