#define ADR_PORT PORTB
#define ADR_DDR DDRB
#define ADR(num) (num)

#define SENSOR_GROUPS_COUNT 2
#define SENSOR_GROUP_DDR DDRB
#define SENSOR_GROUP_PIN PINB
#define SENSOR_GROUP(num) (3 + num)

#define ROOMS_COUNT 4
struct info {
	uint8_t lock : 1; // individual lock for each door, window, motion
	uint8_t group : 4; // max count of MUX's = 16
	uint8_t pin : 3; // max count of MUX pins = 8
};
struct Room {
	struct info door;
	struct info window;
	struct info motionSensor;
	char* pass;
} rooms[ROOMS_COUNT];
void initRooms() {
	rooms[0].door.group = 0;
	rooms[0].door.pin = 0;
	rooms[0].window.group = 0;
	rooms[0].window.pin = 1;
	rooms[0].motionSensor.group = 0;
	rooms[0].motionSensor.pin = 2;
	rooms[0].pass = "1231";

	rooms[1].door.group = 0;
	rooms[1].door.pin = 3;
	rooms[1].window.group = 0;
	rooms[1].window.pin = 4;
	rooms[1].motionSensor.group = 0;
	rooms[1].motionSensor.pin = 5;
	rooms[1].pass = "1232";

	rooms[2].door.group = 0;
	rooms[2].door.pin = 6;
	rooms[2].window.group = 0;
	rooms[2].window.pin = 7;
	rooms[2].motionSensor.group = 1;
	rooms[2].motionSensor.pin = 0;
	rooms[2].pass = "1233";

	rooms[3].door.group = 1;
	rooms[3].door.pin = 1;
	rooms[3].window.group = 1;
	rooms[3].window.pin = 2;
	rooms[3].motionSensor.group = 1;
	rooms[3].motionSensor.pin = 3;
	rooms[3].pass = "1234";
}

//LCD
#define LCD_PORT PORTC
#define LCD_DDR DDRC
// inc only +1: 0(RS), 1(E), 2(D4), 3(D5), ...
#define LCD_RS 0 // LOW - commands, HIGH - data
#define LCD_E 1
#define LCD_D(num) (num - 2) // D4 - 2, D5 - 3, ..., D7 - 5. D4-low byte
#define LCD_BYTE_HIGH 1
#define LCD_BYTE_LOW 0
#define LCD_CHAR_LOCK 0x00
#define LCD_CHAR_UNLOCK 0x01
#define LCD_CHAR_ACCIDENT 0x02

//inc only +1
#define KEY_PIN PIND
#define KEY_PORT PORTD
#define KEY_DDR DDRD
#define KEY_COLUMN(num) (num + 4)
#define KEY_ROW(num) (num)

// functions
void portSetup();
void timerSetup();
void setMuxAdr(uint8_t adr);
uint8_t checkRoom(uint8_t num);
void lockRoom(uint8_t room, uint8_t lock);
uint8_t isRoomLocked(uint8_t room);
void updateInfo();
uint8_t cmpString(char* str1, char* str2, uint8_t len);

char KEY_readPad();

enum LCD_infoChar {
	LOCK,
	DOOR,
	WINDOW,
	MOTION
};
void LCD_init();
void LCD_sendHalfByte(uint8_t byte, uint8_t part);
void LCD_setHalfByte(uint8_t byte, uint8_t part);
void LCD_sendCommand(uint8_t command);
void LCD_sendData(uint8_t data);
void LCD_sendString(char* str, uint8_t len, uint8_t line, uint8_t pos);
void LCD_createChar(uint8_t num, const uint8_t ms[]);
void LCD_setInfo(uint8_t room, enum LCD_infoChar infoChar, char* str);
void LCD_showMenu();
void LCD_clear();

const uint8_t lockChar[8] = {
	0b01110, 
	0b10001, 
	0b10001, 
	0b11111, 
	0b11111, 
	0b11011, 
	0b11111, 
	0b00000
};
const uint8_t unlockChar[8] = {
	0b01110,
	0b10001,
	0b10000,
	0b11111,
	0b11111,
	0b11011,
	0b11111,
	0b00000
};
const uint8_t accidentChar[8] = {
	0b11100,
	0b11100,
	0b11100,
	0b11100,
	0b00000,
	0b11100,
	0b11100,
	0b11100
};
