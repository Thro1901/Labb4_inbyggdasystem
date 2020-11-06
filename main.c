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


char name[9] = "Thomas  ";
char name1[9] = "heter   ";
char message[9] = "jag     ";
char valueFromEEPROM[24];

double* ArrayWithMessages[] = { name, name1, message }; // pointer of char arrays

void main (void) {

	i2c_init();
	uart_init();

	sei();

	eeprom_write_page(ADDR_To_WRITE, ArrayWithMessages[0]);

	eeprom_write_page(ADDR_To_WRITE + strlen(name), ArrayWithMessages[1]);

	eeprom_write_page(ADDR_To_WRITE + strlen(name) + strlen(name1), ArrayWithMessages[2]);

	eeprom_sequential_read(valueFromEEPROM, ADDR_To_WRITE, strlen(name) + strlen(name1) + strlen(message));

	printf_P(PSTR("\nDATA FROM EEPROM: %s "),valueFromEEPROM);

	for (int i = 0; i < strlen(valueFromEEPROM); i++)
	{
		printf_P(PSTR("%x"), valueFromEEPROM[i]);
	}

	

	while (1) {	
		
	}
}

