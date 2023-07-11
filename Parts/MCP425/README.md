# MCP425
Support for the MCP425 digital dual 10K potentiometer by Microchip.
There are other parts in this series that could be supported with minor modification.
This uses an SPI interface.

## Usage
Note that the SPI enable/disable is the responsibility of the user.

The following example initialises the module, and configures channel 0 to 4K7
Also note that the resistance is defined between the wiper terminal and terminal B.

```C
SPI_Init(MCP425_SPI, MCP425_SPI_BITRATE, SPI_Mode_0);
MCP425_Init();
MCP425_SetTerminals(0, MCP425_T_All);
MCP425_SetResistance(0, 4700);
```

## Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
// MCP425 interface
#define MCP425_SPI          SPI_1
#define MCP425_CS_PIN       PA0

// Configure SPI1
#define SPI1_PINS		    (PB3 | PB4 | PB5)
#define SPI1_AF			    GPIO_AF0_SPI1
#define SPI1_ENABLE
```
