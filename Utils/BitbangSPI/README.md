# Bitbang SPI
Bitbangs SPI on some GPIO. Helpful in a pinch.

## Usage

This is intended to be a replacement for the SPI module when required.
Note that the timing is based around the CORE_DelayUs(), so higher bitrates are not going to be achievable.

```C
BSPI_Init(1000000);

uint8_t tx[] = { 0x01, 0x02, 0x03 };
BSPI_Write(tx, sizeof(tx));
uint8_t rx[3];
BSPI_Read(rx, sizeof(rx));
```

Various SPI modes could be implemented, but as of now: only SPI_Mode_0 is implemented.

## Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
#define BSPI_GPIO		GPIOA
#define BSPI_MISO		GPIO_PIN_4
#define BSPI_MOSI		GPIO_PIN_7
#define BSPI_SCK		GPIO_PIN_6
```