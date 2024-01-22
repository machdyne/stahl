/*
 * Text Editor for Stahl
 * Copyright (c) 2021 Lone Dynamics Corporation
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "stahl.h"
#include "editor.h"
#include "fram.h"

#define ROWS 23
#define COLS 80

#define VT100_CURSOR_UP			"\e[A"
#define VT100_CURSOR_DOWN		"\e[B"
#define VT100_CURSOR_RIGHT		"\e[C"
#define VT100_CURSOR_LEFT		"\e[D"
#define VT100_CURSOR_HOME		"\e[;H"
#define VT100_CURSOR_MOVE_TO	"\e[%i;%iH"
#define VT100_CURSOR_CRLF		"\e[E"
#define VT100_CLEAR_HOME		"\e[;H"
#define VT100_ERASE_SCREEN		"\e[J"
#define VT100_ERASE_LINE		"\e[K"

#define CH_ESC		0x1b
#define CH_LF		0x0a
#define CH_CR		0x0d
#define CH_FF		0x0c
#define CH_ACK		0x06
#define CH_BEL		0x07
#define CH_BS		0x08
#define CH_DLE		0x10
#define CH_DEL		0x7f
#define CH_ETB		0x17
#define CH_EM		0x19

#define MODE_EDIT 1
#define MODE_HELP 2

#define STATE_NONE 0
#define STATE_ESC0 1
#define STATE_ESC1 2
#define STATE_ESC2 3

#define KEY_SIZE 256

const PROGMEM char stahl_banner[] =
	"STAHL FIRMWARE V%s\r\n"
	"Copyright (c) 2021 Lone Dynamics Corporation. All rights reserved.\r\n"
	"\r\n";

const PROGMEM char help_password[] =
	"\r\n"
	"If you set a password, all text will be decrypted using that\r\n"
	"password and new text will be encrypted using that password.\r\n"
	"\r\n"
	"When you reconnect the device, press CTRL-P again and re-enter\r\n"
	"your password. Press ENTER to use no encryption/decryption.\r\n"
	"\r\n";

const PROGMEM char help_dfu[] =
	"\r\n"
	"To enter firmware update mode, type UPDATE (in all caps) below,\r\n"
	"and the device will enter USB DFU update mode.\r\n"
	"\r\n"
	"To return to normal mode simply unplug and then plug-in the device.\r\n"
	"\r\n";

const PROGMEM char help_editor[] =
	"\r\n"
	"\r\n"
	"COMMANDS:\r\n"
	"\r\n"
	"CTRL-G    HELP\r\n"
	"CTRL-L    REFRESH SCREEN\r\n"
	"CTRL-W    TOGGLE WRITE MODE\r\n"
	"CTRL-F    TOGGLE FLASHLIGHT MODE\r\n"
	"CTRL-P    SET ENCRYPTION PASSWORD\r\n"
	"CTRL-Y    UPDATE FIRMWARE\r\n";

const PROGMEM char help_press_any_key[] =
	"\r\n"
	"Press any key to return to the editor.\r\n";

uint8_t led_idle = LED_IDLE;

void editor(void);
void editor_set_password(void);
void readline(char *buf, int maxlen);
uint32_t xorshift32(uint32_t state[]);
char editor_encode(int addr, char pc);
char editor_decode(int addr, char cc);
void editor_flashlight(void);
void editor_help(void);
void editor_dfu(void);

#define BOOT_KEY 0xb007b007
#define BOOTLOADER_ADDR 0x7000
//#define BOOTLOADER_ADDR 0x7800
uint32_t boot_key ATTR_NO_INIT;

#ifdef EDITOR_MAIN
int main(int argc, char *argv[]) {

	editor_init();
	while (1) editor_yield();

	return 0;

}
#endif

int mode = MODE_EDIT;
int state = STATE_NONE;
int x, y;

bool write_enabled = false;
bool encryption_enabled = false;
uint8_t key[KEY_SIZE];

void editor_init(void) {

	printf(VT100_CLEAR_HOME);
	printf(VT100_ERASE_SCREEN);
	x = 1;
	y = 1;

	editor_redraw();

}

void editor_redraw(void) {
	char buf[COLS];
	editor_status();
	printf(VT100_CURSOR_MOVE_TO, 24, 59);
	printf("PRESS CTRL-G FOR HELP");

	stahl_led(LED_READ);

	for (int y = 0; y < ROWS; y++) {
		fram_read(buf, y * COLS, COLS);
		printf(VT100_CURSOR_MOVE_TO, y + 1, 1);
		for (int x = 0; x < COLS; x++) {
			char pc = editor_decode(y * COLS + x, buf[x]);
			if (pc > 0x1f && pc < 0x7f)
				putchar(pc);
			else
				putchar('.');
		}
	}

	printf(VT100_CURSOR_MOVE_TO, y, x);
	fflush(stdout);
}

void editor_status(void) {
	printf(VT100_CURSOR_MOVE_TO, 24, 1);
	printf("STAHL -- Line %.2i Column %.2i -- ", y, x);
	if (write_enabled) printf("READ-WRITE"); else printf("READ-ONLY");
	printf("  ");
	printf(VT100_CURSOR_MOVE_TO, y, x);
	fflush(stdout);
	USBTasks();
}

char editor_encode(int addr, char pc) {

	if (encryption_enabled) {
		char k = key[addr % KEY_SIZE];
		return(pc ^ k);
	} else {
		return(pc);
	}

}

char editor_decode(int addr, char cc) {

	if (encryption_enabled) {
		char k = key[addr % KEY_SIZE];
		return(cc ^ k);
	} else {
		return(cc);
	}

}

void editor_flashlight(void) {
	if (led_idle == LED_FLASHLIGHT)
		led_idle = LED_IDLE;
	else
		led_idle = LED_FLASHLIGHT;
}

void editor_help(void) {

	mode = MODE_HELP;

	printf(VT100_CLEAR_HOME);
	printf(VT100_ERASE_SCREEN);

	printf_P(stahl_banner, STAHL_VERSION);

	printf("MEMORY: ");

	if (fram_valid_id()) {
		printf("OK (2KB)\r\n");
	} else {
		printf("INVALID ID; UNABLE TO COMMUNICATE WITH FRAM.\r\n");
	}

	printf("SERIAL: ");
	print_serial_num();

	puts_P(help_editor);
	puts_P(help_press_any_key);

}

void editor_dfu(void) {

	char confirm[8];

	printf(VT100_CLEAR_HOME);
	printf(VT100_ERASE_SCREEN);

	printf_P(help_dfu);
	printf("> ");
	fflush(stdout);
	readline(confirm, 7);

	if (strncmp(confirm, "UPDATE", 7)) {
		mode = MODE_HELP;
		printf("\r\n\r\nNot entering firmware update mode.\r\n");
		puts_P(help_press_any_key);
		return;
	}

	// reset will trigger the bootloader
	USB_Disable();
	cli();
	Delay_MS(1000);
	wdt_enable(WDTO_250MS);
	for (;;);
}

void editor_set_password(void) {

	char password[16];

	printf(VT100_CLEAR_HOME);
	printf(VT100_ERASE_SCREEN);

	puts_P(help_password);

	printf("Password (0-15 characters): ");
	fflush(stdout);
	readline(password, 15);

	printf("\r\n");
	printf("\r\n");

	mode = MODE_HELP;

	if (!strlen(password)) {
		printf("ENCRYPTION DISABLED.\r\n");
		encryption_enabled = false;
	} else {
		printf("SETTING PASSWORD TO: '%s'\r\n", password);
	}

	puts_P(help_press_any_key);

	if (!strlen(password)) return;

	uint32_t key_seed = 0x12345678;
	for (int i = 0; i < strlen(password); i++) {
		key_seed += password[i];
	}

	uint32_t state[1] = { key_seed };

	for (int i = 0; i < KEY_SIZE; i++) {
		key[i] = (uint8_t)(xorshift32(state) & 0xff);
	}

	encryption_enabled = true;


}

uint32_t xorshift32(uint32_t state[]) {
	uint32_t x = state[0];
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	state[0] = x;
	return x;
}

void readline(char *buf, int maxlen) {

	int c;
	int pl = 0;

	memset(buf, 0x00, maxlen + 1);

	while (1) {

		c = getchar();

		if (c == CH_CR)
			return;
		else if (c == CH_BS || c == CH_DEL) {
			pl--;
			buf[pl] = 0x00;
			printf(VT100_CURSOR_LEFT);
			printf(" ");
			printf(VT100_CURSOR_LEFT);
			printf(VT100_CURSOR_LEFT);
		}
		else if (c > 0) {
			putchar(c);
			buf[pl++] = c;
		}

		if (pl < 0) pl = 0;
		if (pl == maxlen) return;

		USBTasks();

	}

}

void editor_yield(void) {

	stahl_led(led_idle);

	int c = getch();
	if (c <= 0) return;

	if (mode == MODE_HELP) {
		mode = MODE_EDIT;
		printf(VT100_ERASE_SCREEN);
		printf(VT100_CLEAR_HOME);
		editor_redraw();
		return;
	}

	switch(c) {

		case(CH_ESC):
			state = STATE_ESC0;
			break;

		case(CH_FF):
			editor_redraw();
			break;

		case('['):
			if (state == STATE_ESC0)
				state = STATE_ESC1;
			break;

		default:
			if (state == STATE_ESC1) {
				state = STATE_NONE;
				if (c == 'A') { y--; printf(VT100_CURSOR_UP); }
				if (c == 'B') { y++; printf(VT100_CURSOR_DOWN); } 
				if (c == 'C') { x++; printf(VT100_CURSOR_RIGHT); }
				if (c == 'D') { x--; printf(VT100_CURSOR_LEFT); }
				if (c == 'H') { x = 1; printf(VT100_CURSOR_HOME); }
				if (c == 'F') { x = COLS; printf(VT100_CURSOR_MOVE_TO, y, x); }
				if (c == '3') {
					if (!write_enabled) break;
					int addr = (y - 1) * COLS + (x - 1);
					stahl_led(LED_WRITE);
					fram_write(addr, editor_encode(addr, 0x00));
					printf(".");
					printf(VT100_CURSOR_LEFT);
					state = STATE_ESC2;
				};
				break;
			}

			if (state == STATE_ESC2) {
				state = STATE_NONE;
				break;
			}

			if (state == STATE_NONE) {
				if (c == CH_BS || c == CH_DEL) {
					if (!write_enabled) break;
					x--;
					int addr = (y - 1) * COLS + (x - 1);
					stahl_led(LED_WRITE);
					fram_write(addr, editor_encode(addr, 0x00));
					printf(VT100_CURSOR_LEFT);
					printf(".");
					printf(VT100_CURSOR_LEFT);
					printf(VT100_CURSOR_LEFT);
					printf(VT100_CURSOR_LEFT);
				} else if (c == CH_DLE) {
					editor_set_password();
				} else if (c == CH_CR) {
					printf(VT100_CURSOR_CRLF);
					x = 1;
					y += 1;
				} else if (c == CH_ACK) {
					editor_flashlight();
				} else if (c == CH_BEL) {
					editor_help();
				} else if (c == CH_EM) {
					editor_dfu();
				} else if (c == CH_ETB) {
					if (write_enabled)
						write_enabled = false;
					else
						write_enabled = true;
				} else {
					if (write_enabled) {
						putchar(c);
						int addr = (y - 1) * COLS + (x - 1);
						stahl_led(LED_WRITE);
						fram_write(addr, editor_encode(addr, c));
					}
					x++;
				}
			}
			break;

	}

	if (x < 1) x = 1;
	if (x > COLS) x = COLS;
	if (y < 1) y = 1;
	if (y > ROWS) y = ROWS;

	editor_status();
	fflush(stdout);

}
