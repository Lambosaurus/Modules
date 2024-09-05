# LED

This module runs a single RGB LED.

This isnt very complex, but I find myself using this frequently.

## Usage

The following code demonstrates this module:

```C
LED_Init();

while (1)
{
    LED_Write(LED_Color_Red);
    CORE_Delay(1000);
    LED_Write(LED_Color_Green);
    CORE_Delay(1000);
    LED_Write(LED_Color_Blue);
    CORE_Delay(1000);
}
```

`LED_PORT_COMMON` may be defined if all the LED pins are on the same GPIO bank, allowing for some minor optimizations.

# Board

The module is dependent on  definitions within `Board.h`

The following template can be used.

```C
// LED config
#define LED_R_PIN           PB5
#define LED_G_PIN           PB6
#define LED_B_PIN           PB7
//#define LED_ACTIVE        GPIO_PIN_RESET
//#define LED_PORT_COMMON
```