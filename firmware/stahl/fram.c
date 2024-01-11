/* 
 * AVR FRAM driver
 * Copyright (c) 2021 Lone Dynamics Corporation
 *
 */

#include <avr/io.h>
#include "fram.h"

#define SPI_SS_FRAM	PC0
#define SPI_SCK		PC1
#define SPI_MOSI		PC2
#define SPI_MISO		PC3

void fram_init(void) {

	DDRB = (1 << SPI_SS_FRAM) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);

	PORTB |= (1 << SPI_SS_FRAM);

}

void fram_read(char *buf, int addr, int len) {

	int i;

	PORTB &= ~(1 << SPI_SS_FRAM);

	spi_xfer(0x03);	// READ

	spi_xfer((addr >> 8) & 0xff);
	spi_xfer(addr & 0xff);

	for (i = 0; i < len; i++)
		buf[i] = spi_xfer(0x00);

	PORTB |= (1 << SPI_SS_FRAM);

}

void fram_write_enable(void) {
	PORTB &= ~(1 << SPI_SS_FRAM);
	spi_xfer(0x06);	// WREN
	PORTB |= (1 << SPI_SS_FRAM);
}

void fram_write(int addr, unsigned char d) {

	fram_write_enable(); // auto-disabled after each write

	PORTB &= ~(1 << SPI_SS_FRAM);

	spi_xfer(0x02);	// WRITE

	spi_xfer((addr >> 8) & 0xff);
	spi_xfer(addr & 0xff);

	spi_xfer(d);

	PORTB |= (1 << SPI_SS_FRAM);

}

bool fram_valid_id(void) {

	bool valid = false;

	PORTB &= ~(1 << SPI_SS_FRAM);

	spi_xfer(0x9f);	// READ ID

	unsigned char man_id = spi_xfer(0x00);
	unsigned char contcode = spi_xfer(0x00);
	unsigned char pidcode1 = spi_xfer(0x00);
	unsigned char pidcode2 = spi_xfer(0x00);

	if (man_id == 0x04 && contcode == 0x7f &&
		pidcode1 == 0x01 && pidcode2 == 0x01) valid = true;

	PORTB |= (1 << SPI_SS_FRAM);

	return valid;

}

unsigned char spi_xfer(unsigned char d) {

	SPDR = d;
	while (!(SPSR & (1 << SPIF)));

	return(SPDR);

}
