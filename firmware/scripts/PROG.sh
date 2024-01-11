# program the bootloader
avrdude -c usbasp -p m32u2 -e -U flash:w:BootloaderDFU.hex -U lock:w:0x0F:m
