# PCF8523
Support for the NXP PCF8523RTC
This uses an I2C inteface.

## Usage
Note that the I2C enable/disable is the responsibility of the user.

The following example initialises the part, and reads out the DateTime.
Initialisation not actually required, and is only present for consistency.

```C
I2C_Init(PCF8523_I2C, I2C_Mode_Fast);
if (PCF8523_Init())
{
    DateTime_t dt;
    if (PCF8523_Read(&dt))
    {
        ...
    }
}
```

## Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
// PCF8523 interface
#define PCF8523_I2C			I2C_1

// Configure I2C1
#define I2C1_PINS		(PB6 | PB7)
#define I2C1_AF			GPIO_AF1_I2C1
```