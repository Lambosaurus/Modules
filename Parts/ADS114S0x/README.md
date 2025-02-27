# ADS114S0x
This implements support for the ADS114S06 and ADS114S08 ADC

## Usage

This module does not take responsibility for init/deinit of the SPI peripheral.

```C
SPI_Init(ADS114S_SPI, SPI_Mode_1, 8000000);
ADS114S_Init();

while (1)
{
    // Read value from channel 0
    int16_t value = ADS114S_Read(0);
    ...
}
```

## Board

The module is dependant on  definitions within `Board.h`
The following template can be used.

```C
// Configure SPI1
#define SPI1_PINS		    (PA5 | PA6 | PA7)
#define SPI1_AF				GPIO_AF0_SPI1

// ADS114S config
#define ADS114S_CS_PIN		PA0
#define ADS114S_SPI			SPI_1
#define ADS114S_MISO_PIN	PA6
```