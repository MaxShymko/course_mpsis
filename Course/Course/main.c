/*
 * Course.c
 *
 * Created: 05.04.2018 11:53:53
 * Author : Prolific
 */ 

#define F_CPU 1000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "globals.h"

ISR(TIMER0_OVF_vect) {
	cli();
	char key = KEY_readPad();
	if (key == '*') {
		LCD_clear();
		LCD_sendString("-Setup mode-", 12, 0, 4);
		LCD_sendString("Choose a room: ", 15, 1, 0);
		int room = -1;
		uint8_t passSymbCount = 0;
		char* pass = "";
		char* str = "";
		while (1) {
			key = KEY_readPad();
			if (key == '\0')
				continue;
			if (key == '#') { // exit
				LCD_clear();
				LCD_showMenu();
				return;
			}
			
			for (uint8_t i = 0; i < ROOMS_COUNT; i++) {
				if (key == i + 1 + '0') {
					room = i;
					str[0] = key;
					LCD_sendString(str, 1, 1, 15);
					LCD_sendString("Password: ", 10, 2, 0);
					_delay_ms(500);
					while (passSymbCount < 4) {
						key = KEY_readPad();
						if (key < '0' || key > '9')
							continue;
						pass[passSymbCount++] = key;
						LCD_sendString("*", 1, 2, 9 + passSymbCount);
						_delay_ms(500);
					}
					if (!cmpString(pass, rooms[room].pass, 4)) { //incorrect pass
						LCD_sendString(" Incorrect password ", 20, 3, 0);
						_delay_ms(2000);
						LCD_clear();
						LCD_showMenu();
						return;
					}
					LCD_sendString("Status:              ", 20, 3, 0);
					char* locked = "LOCKED   ";
					locked[9] = LCD_CHAR_LOCK;
					char* unlocked = "UNLOCKED ";
					unlocked[9] = LCD_CHAR_UNLOCK;
					uint8_t status = isRoomLocked(room);
					LCD_sendString(status ? locked : unlocked, 10, 3, 8);
					while (1) {
						key = KEY_readPad();
						if (key == '\0')
							continue;
						if (key == '#') {// exit
							LCD_clear();
							LCD_showMenu();
							return;
						}
						if (key == '*') {
							status = !status;
							lockRoom(room, status);
							LCD_sendString(status ? locked : unlocked, 10, 3, 8);
							_delay_ms(300);
						}
					}
				}
			}
		}
	}
	sei();
}

int main(void)
{
	portSetup();
	timerSetup();
	initRooms(rooms);
	LCD_init();

	for (uint8_t i = 0; i < ROOMS_COUNT; i++) lockRoom(i, 0); // UNLOCK rooms

    while (1) {
		cli();
		updateInfo();
		sei();
    }
}

uint8_t cmpString(char* str1, char* str2, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		if (str1[i] != str2[i])
			return 0;
	}
	return 1;
}

void portSetup() {
	//rooms
	ADR_DDR |= (1 << ADR(0)) | (1 << ADR(1)) | (1 << ADR(2)); // to port out
	ADR_PORT &= ~(1 << ADR(0)) & ~(1 << ADR(1)) & ~(1 << ADR(2)); // clear adr
	for (uint8_t i = 0; i < SENSOR_GROUPS_COUNT; i++) {
		SENSOR_GROUP_DDR &= ~(1 << SENSOR_GROUP(i));
	}
	
	//LCD
	LCD_DDR |= (1 << LCD_RS) | (1 << LCD_E) | (1 << LCD_D(4)) | (1 << LCD_D(5)) | (1 << LCD_D(6)) | (1 << LCD_D(7));
	LCD_PORT = 0x00;
	
	//KEY
	KEY_DDR |= (1 << KEY_COLUMN(0)) | (1 << KEY_COLUMN(1)) | (1 << KEY_COLUMN(2));
	KEY_DDR &= ~(1 << KEY_ROW(0)) & ~(1 << KEY_ROW(1)) & ~(1 << KEY_ROW(2)) & ~(1 << KEY_ROW(3));
	KEY_PORT |= 0x7F;
}

void timerSetup() {
	TIMSK0 |= (1 << TOIE0); // Overflow interrupt
	TCCR0B |= (1 << CS02); // F/256
}

void lockRoom(uint8_t room, uint8_t lock) {
	rooms[room].door.lock = lock;
	rooms[room].window.lock = lock;
	rooms[room].motionSensor.lock = lock;
}

uint8_t isRoomLocked(uint8_t room) {
	return rooms[room].door.lock;
}

uint8_t checkRoom(uint8_t num) {
	uint8_t result = 0;
	
	setMuxAdr(rooms[num].door.pin);
	if ((SENSOR_GROUP_PIN & (1 << SENSOR_GROUP(rooms[num].door.group))) == 0)
		result |= 1;
	
	setMuxAdr(rooms[num].window.pin);
	if ((SENSOR_GROUP_PIN & (1 << SENSOR_GROUP(rooms[num].window.group))) == 0)
		result |= 2;
		
	setMuxAdr(rooms[num].motionSensor.pin);
	if ((SENSOR_GROUP_PIN & (1 << SENSOR_GROUP(rooms[num].motionSensor.group))) != 0)
		result |= 4;
	
	return result;
}

void setMuxAdr(uint8_t adr) {
	if (adr & 0b001) ADR_PORT |= (1 << ADR(0));
	else ADR_PORT &= ~(1 << ADR(0));
	if (adr & 0b010) ADR_PORT |= (1 << ADR(1));
	else ADR_PORT &= ~(1 << ADR(1));
	if (adr & 0b100) ADR_PORT |= (1 << ADR(2));
	else ADR_PORT &= ~(1 << ADR(2));
}

void updateInfo() {
	uint8_t roomStatus;
	char* str = " ";
	for (uint8_t i = 0; i < ROOMS_COUNT; i++) {
		roomStatus = checkRoom(i);
		LCD_setInfo(i, DOOR, (roomStatus & 1) ? "O" : "C");
		LCD_setInfo(i, WINDOW, (roomStatus & 2) ? "O" : "C");
		LCD_setInfo(i, MOTION, (roomStatus & 4) ? "Y" : "N");
		
		if ((rooms[i].door.lock && (roomStatus & 1)) ||
			(rooms[i].window.lock && (roomStatus & 2)) ||
			(rooms[i].motionSensor.lock && (roomStatus & 4))) {
			str[0] = LCD_CHAR_ACCIDENT;
			LCD_setInfo(i, LOCK, str);
		} else {
			// LOCK - ON if all sensors are locked
			str[0] = (rooms[i].door.lock && rooms[i].window.lock && rooms[i].motionSensor.lock) ? LCD_CHAR_LOCK : LCD_CHAR_UNLOCK;
			LCD_setInfo(i, LOCK, str);
		}
	}
}

void LCD_init() {
	_delay_ms(100);
	LCD_sendHalfByte(0b0011, LCD_BYTE_LOW);
	_delay_us(100);
	LCD_sendHalfByte(0b0011, LCD_BYTE_LOW);
	_delay_us(100);
	LCD_sendHalfByte(0b0011, LCD_BYTE_LOW);
	_delay_us(100);
	LCD_sendHalfByte(0b0010, LCD_BYTE_LOW);
	_delay_us(100);
	
	LCD_sendCommand(0b00101000);
	LCD_sendCommand(0b00001000);
	LCD_sendCommand(0b00000001);
	_delay_ms(150);
	LCD_sendCommand(0b00000110);
	
	LCD_sendCommand(0b00001100); // cursor
	
	LCD_createChar(LCD_CHAR_LOCK, lockChar);
	LCD_createChar(LCD_CHAR_UNLOCK, unlockChar);
	LCD_createChar(LCD_CHAR_ACCIDENT, accidentChar);
	
	LCD_showMenu();
}

void LCD_clear() {
	LCD_sendCommand(0b00000001);
	_delay_ms(150);
}

void LCD_showMenu() {
	char* string = "-RM:-, D:-, W:-, M:-";
	for (uint8_t i = 0; i < ROOMS_COUNT; i++) {
		string[4] = '0' + i + 1;
		LCD_sendString(string, 20, i, 0);
	}
}

void LCD_sendString(char* str, uint8_t len, uint8_t line, uint8_t pos) {
	switch (line) {
		case 0: pos += 0x00;break;
		case 1: pos += 0x40;break;
		case 2: pos += 0x14;break;
		case 3: pos += 0x54;break;
	}
	LCD_sendCommand(0x80 | pos);
	for (uint8_t i = 0; i < len; i++) {
		LCD_sendData(str[i]);
	}
}

void LCD_sendData(uint8_t data) {
	LCD_PORT |= (1 << LCD_RS);
	LCD_sendCommand(data);
	LCD_PORT &= ~(1 << LCD_RS);
}

void LCD_sendCommand(uint8_t command) {
	LCD_sendHalfByte(command, LCD_BYTE_HIGH);
	LCD_sendHalfByte(command, LCD_BYTE_LOW);
}

void LCD_sendHalfByte(uint8_t byte, uint8_t part) {
	LCD_PORT |= (1 << LCD_E);
	LCD_setHalfByte(byte, part);
	LCD_PORT &= ~(1 << LCD_E);
	_delay_us(100);
}

void LCD_setHalfByte(uint8_t byte, uint8_t part) {
	for (uint8_t i = 0; i < 4; i++) {
		if (byte & (1 << (i + part*4))) LCD_PORT |= (1 << LCD_D(i + 4));
		else LCD_PORT &= ~(1 << LCD_D(i + 4));
	}
}

void LCD_createChar(uint8_t num, const uint8_t ms[]) {
	uint8_t adr = num * 8;
	LCD_sendCommand(0x40 | adr);
	for (uint8_t i = 0; i < 8; i++, adr++) {
		LCD_sendData(ms[i]);
	}
}

void LCD_setInfo(uint8_t room, enum LCD_infoChar infoChar, char* str) {
	switch (infoChar) {
		case LOCK:
			LCD_sendString(str, 1, room, 0);
			return;
		case DOOR:
			LCD_sendString(str, 1, room, 9);
			return;
		case WINDOW:
			LCD_sendString(str, 1, room, 14);
			return;
		case MOTION:
			LCD_sendString(str, 1, room, 19);
	};
}

char KEY_readPad() {
	char res = '\0';
	KEY_PORT |= (1 << KEY_COLUMN(0)) | (1 << KEY_COLUMN(1)) | (1 << KEY_COLUMN(2));
	for (uint8_t i = 0; i < 3; i++) {
		KEY_PORT &= ~(1 << KEY_COLUMN(i));
		
		for (uint8_t j = 0; j < 4; j++) {
			if (KEY_PIN & (1 << KEY_ROW(j)))
				continue;
			if (j == 3) {
				switch (i) {
					case 0: res = '*'; break;
					case 1: res = '0'; break;
					case 2: res = '#'; break;
				}
			} else {
				res = j * 3 + i + 1 + '0';
			}
		}
		
		KEY_PORT |= (1 << KEY_COLUMN(i));
	}
	return res;
}
