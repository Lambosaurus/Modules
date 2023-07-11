# SHTC3
Support for the Sensiron SHTC3 temperature and humidity sensor.
This uses an I2C inteface.

## Usage
Note that the SPI enable/disable is the responsibility of the user.

The following example initialises the part, and reads out the temp & humidity.

```C
I2C_Init(SHTC3_I2C, I2C_Mode_Fast);
if (SHTC3_Init())
{
    int16_t temp; // In deci-degrees
    uint8_t hum;   // In percent
    if (SHTC3_Read(&temp, &hum))
    {
        ...
    }
}
```

## I2C Timeout

This part is dependant on clock stretching during the read process.
You should increase the `I2C_TIMEOUT` to at least 20 ms.

## Board

The module is dependant on  definitions within `Board.h`

The following template can be used.


```C
// SHTC3 interface
#define SHTC3_I2C       I2C_1

// Configure I2C1
#define I2C1_PINS		(PB6 | PB7)
#define I2C1_AF			GPIO_AF1_I2C1
#define I2C_TIMEOUT     20
```