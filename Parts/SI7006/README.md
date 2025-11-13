# SI7006
Support for the Silabs SI7006 temperature & humidity sensor
This uses an I2C inteface.

## Usage
Note that the I2C enable/disable is the responsibility of the user.

The following example initialises the part, and reads out the pressure.

```C
I2C_Init(SI7006_I2C, I2C_Mode_Fast);
if (SI7006_Init(SDP8XX_Mode_Cont))
{
    int16_t temp; // deci-deg
    uint8_t hum;  // % rh
    if (SI7006_Read(&temp, &hum))
    {
        ...
    }
}
```

## Board

The module is dependant on  definitions within `Board.h`

The following template can be used.


```C
// SDP8XX interface
#define SI7006_I2C		I2C_1

// Configure I2C1
#define I2C1_PINS		(PB6 | PB7)
#define I2C1_AF			GPIO_AF1_I2C1
```