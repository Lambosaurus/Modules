# LCD
Controls a basic LCD character display based around the HD44780 controller.
This controller is extremely common in 2x16 character LCD displays.

This uses the part in 4 bit mode without read control.

## Usage

```C
LCD_Init();

// Print "Title" on line 0, position 0.
LCD_Seek(0, 0);
LCD_WriteStr("Title");

int seconds = 0;

while (1)
{
    // Write a line to the LCD.
    char line[LCD_LINE_SIZE+1];
    snprintf(line, sizeof(line), "T: %d", seconds);
    LCD_Seek(1, 0);
    LCD_WriteStr(line);

    seconds += 1;
    CORE_Delay(1000);
}
```
## Pins

Some notes on the pin configuration:

 * The `RW` pin should be tied low.
 * `D4` to `D7` are used in 4 bit mode. `D0` to `D3` are unused.
 * `LCD_PWR_PIN` can be optionally supplied for logic or blacklight power control

## Board

The module is dependant on definitions within `Board.h`

```C
// LCD interface
//#define LCD_PWR_PIN			PA15
#define LCD_EN_PIN			PB4
#define LCD_RS_PIN			PB3
#define LCD_D4_PIN			PB5
#define LCD_D5_PIN			PB6
#define LCD_D6_PIN			PB7
#define LCD_D7_PIN			PB8
```
