
#include "LCD.h"

#include "GPIO.h"
#include "CORE.h"
#include "US.h"

/*
 * PRIVATE DEFINITIONS
 */

// commands
#define LCD_CLEARDISPLAY 		0x01
#define LCD_RETURNHOME 			0x02
#define LCD_ENTRYMODESET 		0x04
#define LCD_DISPLAYCONTROL 		0x08
#define LCD_CURSORSHIFT 		0x10
#define LCD_FUNCTIONSET 		0x20
#define LCD_SETCGRAMADDR	 	0x40
#define LCD_SETDDRAMADDR 		0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 			0x00
#define LCD_ENTRYLEFT 			0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 			0x04
#define LCD_DISPLAYOFF 			0x00
#define LCD_CURSORON 			0x02
#define LCD_CURSOROFF 			0x00
#define LCD_BLINKON 			0x01
#define LCD_BLINKOFF 			0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 		0x08
#define LCD_CURSORMOVE 			0x00
#define LCD_MOVERIGHT 			0x04
#define LCD_MOVELEFT 			0x00

// flags for function set
#define LCD_8BITMODE 			0x10
#define LCD_4BITMODE 			0x00
#define LCD_2LINE 				0x08
#define LCD_1LINE 				0x00
#define LCD_5x10DOTS 			0x04
#define LCD_5x8DOTS 			0x00

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */


static void LCD_WriteNibble(uint8_t nibble);
static void LCD_WriteChar(uint8_t word);
static void LCD_Command(uint8_t word);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void LCD_Init(void)
{
#ifdef LCD_PWR_PIN
	GPIO_EnableOutput(LCD_PWR_PIN, GPIO_PIN_SET);
#endif
	GPIO_EnableOutput(LCD_D4_PIN | LCD_D5_PIN | LCD_D6_PIN | LCD_D7_PIN, GPIO_PIN_RESET);
	GPIO_EnableOutput(LCD_EN_PIN, GPIO_PIN_RESET);
	GPIO_EnableOutput(LCD_RS_PIN, GPIO_PIN_RESET);

	CORE_Delay(50);

    // Sequence required to enter 4 bit mode
	LCD_WriteNibble(0x03);
    CORE_Delay(5);
    LCD_WriteNibble(0x03);
    CORE_Delay(5);
    LCD_WriteNibble(0x03);
    US_Delay(150);
    LCD_WriteNibble(0x02);

    LCD_Command(LCD_FUNCTIONSET | LCD_5x8DOTS | LCD_2LINE | LCD_4BITMODE);
    LCD_Command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    LCD_Clear();
    LCD_Command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
}

void LCD_Deinit(void)
{
	GPIO_Deinit(LCD_D4_PIN | LCD_D5_PIN | LCD_D6_PIN | LCD_D7_PIN);
	GPIO_Deinit(LCD_EN_PIN);
	GPIO_Deinit(LCD_RS_PIN);
#ifdef LCD_PWR_PIN
	GPIO_Write(LCD_PWR_PIN, GPIO_PIN_RESET);
#endif
}

void LCD_Clear(void)
{
	LCD_Command(LCD_CLEARDISPLAY);
	CORE_Delay(2);
}

void LCD_Seek(uint8_t row, uint8_t col)
{
	uint8_t address = (row * 0x40) + col;
	LCD_Command(LCD_SETDDRAMADDR | address);
}

void LCD_Write(const uint8_t * data, uint32_t size)
{
	while (size--)
	{
		LCD_WriteChar(*data++);
	}
}

void LCD_WriteStr(const char * str)
{
	LCD_Write((uint8_t*)str, strlen(str));
}

/*
 * PRIVATE FUNCTIONS
 */

static void LCD_WriteNibble(uint8_t nibble)
{
	GPIO_Write(LCD_D4_PIN, nibble & 0x01);
	GPIO_Write(LCD_D5_PIN, nibble & 0x02);
	GPIO_Write(LCD_D6_PIN, nibble & 0x04);
	GPIO_Write(LCD_D7_PIN, nibble & 0x08);

	GPIO_Write(LCD_EN_PIN, GPIO_PIN_RESET);
	US_Delay(1);
	GPIO_Write(LCD_EN_PIN, GPIO_PIN_SET);
	US_Delay(1);    // enable pulse must be >450 ns
	GPIO_Write(LCD_EN_PIN, GPIO_PIN_RESET);
	US_Delay(100);   // commands need >37 us to settle
}

static void LCD_WriteChar(uint8_t word)
{
	GPIO_Write(LCD_RS_PIN, GPIO_PIN_SET);
	LCD_WriteNibble(word >> 4);
	LCD_WriteNibble(word);
}

static void LCD_Command(uint8_t word)
{
	GPIO_Write(LCD_RS_PIN, GPIO_PIN_RESET);
	LCD_WriteNibble(word >> 4);
	LCD_WriteNibble(word);
}


/*
 * INTERRUPT ROUTINES
 */

