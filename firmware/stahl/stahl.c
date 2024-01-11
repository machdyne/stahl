/*
  Stahl Firmware

  Copyright 2021  Dean Camera (dean [at] fourwalledcubicle [dot] com)
  Copyright 2021  Lone Dynamics Corporation

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "stahl.h"

USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
				.DataINEndpoint           =
					{
						.Address          = CDC_TX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint =
					{
						.Address          = CDC_RX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.NotificationEndpoint =
					{
						.Address          = CDC_NOTIFICATION_EPADDR,
						.Size             = CDC_NOTIFICATION_EPSIZE,
						.Banks            = 1,
					},
			},
	};

static FILE USBSerialStream;

bool hostReady = 0;
int cbyte = -1;

void stahl_led(uint8_t intensity)
{
	if (intensity == 0) {
		TCCR1A = 0;
		PORTC = 0;
		return;
	}
	OCR1B = intensity;
	TCCR1A = (1 << COM1B1) | (1 << WGM12) | (1 << WGM10);	// fast pwm 8-bit
	TCCR1B = (1 << CS11);	// clk/8
}

int main(void)
{
	SetupHardware();

	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
	stdout = &USBSerialStream;
	stdin = &USBSerialStream;

	// set LED as output
	DDRC |= (1 << 5);
	stahl_led(LED_STARTUP);

	GlobalInterruptEnable();

	editor_init();

	for (;;)
	{
		editor_yield();
		cbyte = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		USBTasks();
	}

}

void USBTasks(void) {
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();
}

int getch(void) {
	return cbyte;
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
//	LEDs_Init();
	USB_Init();
	fram_init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

/** CDC class driver callback function the processing of changes to the virtual
 *  control lines sent from the host..
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo)
{
	/* You can get changes to the virtual CDC lines in this callback; a common
	   use-case is to use the Data Terminal Ready (DTR) flag to enable and
	   disable CDC communications in your application when set to avoid the
	   application blocking while waiting for a host to become ready and read
	   in the pending data from the USB endpoints.
	*/
	hostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;

	editor_redraw();

}

void print_serial_num(void) {

	for (int i = 0; i < 3; i++)
		printf("%.2x", boot_signature_byte_get(0x0 + i));

	printf("-");

	for (int i = 0; i < 10; i++)
		printf("%.2x", boot_signature_byte_get(0x0E + i));

}
