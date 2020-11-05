#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "serial.h"
#include "timer.h"


char name[10];


void main (void) {

	i2c_init();
	uart_init();

	sei();
	eeprom_write_byte(ADDR_To_WRITE, 'T');
	eeprom_wait_until_write_complete();
	eeprom_write_byte(0x71, 'H');
	eeprom_wait_until_write_complete();
	eeprom_write_byte(0x72, 'O');
	eeprom_wait_until_write_complete();
	eeprom_write_byte(0x73, 'M');
	eeprom_wait_until_write_complete();
	eeprom_write_byte(0x74, 'A');
	eeprom_wait_until_write_complete();
	eeprom_write_byte(0x75, 'S');
	eeprom_wait_until_write_complete();


	name[0] = eeprom_read_byte(ADDR_To_WRITE);
	name[1] = eeprom_read_byte(0x71);
	name[2] = eeprom_read_byte(0x72);
	name[3] = eeprom_read_byte(0x73);
	name[4] = eeprom_read_byte(0x74);
	name[5] = eeprom_read_byte(0x75);

	printf_P(PSTR("DATA FROM EEPROM: %s \n"), name);


	while (1) {	
		
	}
}

