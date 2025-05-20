# 24LC04
This implements support for the TCA9548A I2C.

This is probably compatabile with the PCA9548 and similar parts.

## Usage

This module does not take responsibility for init/deinit of the I2C peripheral.

```C
I2C_Init(TCA9548_I2C, I2C_Mode_Fast);

// Select port 3 on the TCA9548A
TCA9548_Select(1 << 3);

// Disable all ports
TCA9548_Select(0);
```

The select takes a bitmask of enabled ports, and any number can be enabled simultanously.

## Board

The module is dependant on  definitions within `Board.h`
The following template can be used.

```C
// Configure I2C1
#define I2C1_PINS		    (PB6 | PB7)
#define I2C1_AF			    GPIO_AF1_I2C1

// TCA9548 config
#define TCA9548_I2C	        I2C_1
```