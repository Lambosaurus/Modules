# SDP8XX
Support for the Sensiron SDP8xx pressure sensor
This uses an I2C inteface.

## Usage
Note that the I2C enable/disable is the responsibility of the user.

The following example initialises the part, and reads out the pressure.

```C
I2C_Init(SDP8XX_I2C, I2C_Mode_Fast);
if (SDP8XX_Init())
{
    int32_t pa // In millipa
    if (SDP8XX_Read(&pa))
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
#define SDP8XX_I2C		I2C_1

// Configure I2C1
#define I2C1_PINS		(PB6 | PB7)
#define I2C1_AF			GPIO_AF1_I2C1
```

## Dependencies

This is dependent on the [CRCX](../../Utils/CRCX) module for its CRC8 implementation.