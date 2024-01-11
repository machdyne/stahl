#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>

int main(void) {
 
 	// disable wdt
   MCUSR &= ~(1 << WDRF);
   wdt_disable();

   /* Disable clock division */
   clock_prescale_set(clock_div_1);

	DDRC = 0xff;
	PORTC = 0x00;

	while (1) {

		PORTC = 0xff;
		_delay_ms(1000);
		PORTC = 0x00;
		_delay_ms(1000);
 
	}

}
