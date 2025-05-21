# VEML3328
This implements support for the Vishay VEML3328 color sensor.

## Usage

This module does not take responsibility for init/deinit of the I2C peripheral.

```C
I2C_Init(VEML3328_I2C, I2C_Mode_Fast);

VEML3328_Init(VEML3328_Gain_1x);

VEML3328_Values_t lum;
VEML3328_Read(&lum);
```

Once initialized, the VEML3328 is left in continous mode, and reads will retrieve the last value.

## Board

The module is dependant on  definitions within `Board.h`
The following template can be used.

```C
// Configure I2C1
#define I2C1_PINS		    (PB6 | PB7)
#define I2C1_AF			    GPIO_AF1_I2C1

// VEML3328 config
#define VEML3328_I2C	       I2C_1
```