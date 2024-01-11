MCU=atmega32u2
F_CPU=8000000UL

blinky:
	avr-gcc -Wall -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -c blinky.c
	avr-gcc -g -mmcu=$(MCU) -o blinky.elf blinky.o
	avr-objdump -S --disassemble blinky.elf > blinky.dasm
	avr-objcopy -O ihex -j .text -j .data blinky.elf blinky.hex

prog:
	sudo dfu-programmer atmega32u2 erase
	sudo dfu-programmer atmega32u2 flash blinky.hex

clean:
	rm -f *.o *.hex *.elf *.dasm
