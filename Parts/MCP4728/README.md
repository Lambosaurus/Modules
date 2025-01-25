# MCP4728
Support for the Microchip MCP4728 4 channel DAC.
This uses an I2C inteface.

## Usage
Note that the I2C enable/disable is the responsibility of the user.

The following example initialises the part, and writes to each of the DAC channels.
Note that when using the 4096mV reference, its equivalent to a 1mV resolution.

```C
I2C_Init(MCP4728_I2C, I2C_Mode_Fast);
if (MCP4728_Init())
{
    // Write to the channels.
    MCP4728_Write(0, MCP4728_Ref_4096mV, 1000);
    MCP4728_Write(1, MCP4728_Ref_4096mV, 2000);
    MCP4728_Write(2, MCP4728_Ref_4096mV, 3000);
    MCP4728_Write(3, MCP4728_Ref_4096mV, 4000);
}
```

## Multiple devices

Muliple MCP4728 devices may be present on the same I2C bus. Support for this is enabled with `MCP4728_SUPPORT_ADDRESSING`.

Selecting the targeted part can be done with `MCP4728_Select`.

While the parts may be ordered with addresses set by the manufacturer - its likely you'll get them all with the default address.
In this case you'll need to control the LDAC pin and change the addresses. A warning that this is done by bit-bashing the I2C bus, due to the silly timing requirements.

The following code shows an example of initialising a device and correcting its address.

```C
// Select the second device
MCP4728_Select(1);

// Try to init the selected device
if (!MCP4728_Init())
{
    // Init failed.
    // The device is probably still on the default address.
    MCP4728_Select(0);
    MCP4728_SetAddress(1, PA1); // LDAC pin for device 1 is on PA1.

    // Device should now be accessable under the desired address.
    MCP4728_Init();
}
```

## Board

The module is dependant on definitions within `Board.h`

The following template can be used.

```C
// MCP4728 interface
#define MCP4728_I2C			I2C_1
//#define MCP4728_SUPPORT_ADDRESSING
//#define MCP4728_SCL_PIN		PB7
//#define MCP4728_SDA_PIN		PB6

// Configure I2C1
#define I2C1_PINS		(PB6 | PB7)
#define I2C1_AF			GPIO_AF1_I2C1
```