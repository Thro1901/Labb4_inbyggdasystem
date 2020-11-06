#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <string.h>

#include "i2c.h"


// Så Control code = 1010, sen Ground för nc,nc,nc = 000, = 10100000. Sen en start bit i början och antigen 1 bit för read och 0 för write.


void i2c_init(void) {

	TWCR = (1 << TWEN);     //enable TWI (Two Wire Interface)
	TWBR = 72;  //will result in the clock speed being 100kHz
	TWSR = 0; // prescale 1

	//SCLfreq = F_CPU/(16+(2TWBRPrescaler))
	//100 000 = 16 000 000 / (16 + (2 * TWBR * 1)
}

void i2c_meaningful_status(uint8_t status) {
	switch (status) {
		case 0x08: // START transmitted, proceed to load SLA+W/R
			printf_P(PSTR("START\n"));
			break;
		case 0x10: // repeated START transmitted, proceed to load SLA+W/R
			printf_P(PSTR("RESTART\n"));
			break;
		case 0x38: // NAK or DATA ARBITRATION LOST
			printf_P(PSTR("NOARB/NAK\n"));
			break;
		// MASTER TRANSMIT
		case 0x18: // SLA+W transmitted, ACK received
			printf_P(PSTR("MT SLA+W, ACK\n"));
			break;
		case 0x20: // SLA+W transmitted, NAK received
			printf_P(PSTR("MT SLA+W, NAK\n"));
				break;
		case 0x28: // DATA transmitted, ACK received
			printf_P(PSTR("MT DATA+W, ACK\n"));
			break;
		case 0x30: // DATA transmitted, NAK received
			printf_P(PSTR("MT DATA+W, NAK\n"));
			break;
		// MASTER RECEIVE
		case 0x40: // SLA+R transmitted, ACK received
			printf_P(PSTR("MR SLA+R, ACK\n"));
			break;
		case 0x48: // SLA+R transmitted, NAK received
			printf_P(PSTR("MR SLA+R, NAK\n"));
			break;
		case 0x50: // DATA received, ACK sent
			printf_P(PSTR("MR DATA+R, ACK\n"));
			break;
		case 0x58: // DATA received, NAK sent
			printf_P(PSTR("MR DATA+R, NAK\n"));
			break;
		default:
			printf_P(PSTR("N/A %02X\n"), status);
			break;
	}
}

inline void i2c_start() {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // Send START condition...
	while ((TWCR & (1 << TWINT)) == 0); //until TWINT resets to zero

								     // Wait for TWINT Flag set. This indicates
									 // that the DATA has been transmitted, and
									 // ACK / NACK has been received
}

inline void i2c_stop() {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);   //Transmit STOP condition.
	while ((TWCR & (1 << TWSTO)));
}

inline uint8_t i2c_get_status(void) {
	uint8_t status;
	//mask status
	status = (TWSR & 0xF8); // reading the five last bits. 11111 000. 000 = prescalebits
	return status;
}

inline void i2c_xmit_addr(uint8_t address, uint8_t rw) {
	TWDR = (address & 0xfe) | (rw & 0x01); //SLA_W		Adress = 0xA0 aka 1010 0000 << RW
	TWCR = (1 << TWINT)|(1 << TWEN); //clear interrupt to start transmission 
	while (!(TWCR & (1 << TWINT)));                      // 0 = write 1 = read
}

inline void i2c_xmit_byte(uint8_t data) {
	TWDR = data;   // It writes a data byte to TWDR register which is shifted to SDA line. 
	TWCR = (1 << TWINT) | (1 << TWEN); 
	while (!(TWCR & (1 << TWINT)));
}

inline uint8_t i2c_read_ACK() {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);  /* Enable TWI(Two Wire Interface), generation of ack and set interrupt high */
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

inline uint8_t i2c_read_NAK() {  
	TWCR = (1 << TWINT) | (1 << TWEN);  /* Enable TWI (Two Wire Interface) and set interrupt high */
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

inline void eeprom_wait_until_write_complete() {
	while (i2c_get_status() != 0x18) { // wait to "SLA+W transmitted, ACK received"
		i2c_start();
		i2c_xmit_addr(EEPROM_ADDR, I2C_W);
	}
}

uint8_t eeprom_read_byte(uint8_t addr) {
	uint8_t data;

	i2c_start();
	i2c_meaningful_status(i2c_get_status());
	// transmitt_adress() + write
	i2c_xmit_addr(EEPROM_ADDR, I2C_W);
	// checka ack
	i2c_meaningful_status(i2c_get_status());
	// WORD ADRESS
	i2c_xmit_byte(addr);
	// checka ack
	i2c_meaningful_status(i2c_get_status());
	i2c_start();
	// transmit_adrress() + 1
	i2c_xmit_addr(EEPROM_ADDR, I2C_R);
		// checka ack
	i2c_meaningful_status(i2c_get_status());

	data = i2c_read_NAK();
	// no ack
	i2c_meaningful_status(i2c_get_status());
	i2c_stop();
	
	return data;
}

void eeprom_write_byte(uint8_t addr, uint8_t data) {
	i2c_start();
	i2c_xmit_addr(EEPROM_ADDR, I2C_W);  // Send controll adress with write "0"
	i2c_xmit_byte(addr);				// Send address to where i want to store the data
	i2c_xmit_byte(data);				// send data
	i2c_stop();							// stop condition
}



void eeprom_write_page(uint8_t addr, uint8_t *data) {
	i2c_start();
	i2c_xmit_addr(EEPROM_ADDR, I2C_W);  // Send controll adress with write "0"
	i2c_xmit_byte(addr);
	for (int i = 0; i < strlen(data); i++)  // lenght of the string(data)
	{	
		i2c_xmit_byte(*data++); // send data and increment to next bit
		
	}

	i2c_stop();
	eeprom_wait_until_write_complete(); // to get a ACK
}

void eeprom_sequential_read(uint8_t *buf, uint8_t start_addr, uint8_t len) {
	
	i2c_start();
	i2c_xmit_addr(EEPROM_ADDR, I2C_W); 	// transmitt_adress() + write
	i2c_xmit_byte(start_addr); 	// WORD ADRESS
	i2c_start();
	i2c_xmit_addr(EEPROM_ADDR, I2C_R); // transmit_adrress() + 1

	for (int i = 0; i < len -1 ; i++)
	{
		*buf++ = i2c_read_ACK();	
	}
	*buf = i2c_read_NAK(); // no ack
	
	i2c_stop();
}

