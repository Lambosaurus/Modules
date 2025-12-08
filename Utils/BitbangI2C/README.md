# Bitbang I2C
Bitbangs I2C on some GPIO. Helpful in a pinch.

## Usage

This is intended to be a replacement for the I2C module when required.
Note that the timing is based around the US_Delay(), so higher bitrates are not going to be achievable.

This also does not (yet) support clock stretching.


```C
BI2C_Init(10000);

uint8_t tx[] = { 0x00 };
uint8_t rx[2];

if (BI2C_Transfer(0x50, tx, sizeof(tx), rx, sizeof(rx)))
{
    // Read 3 bytes from i2c device 0x50.
}
```

## Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
// Configure bit banged I2C
#define BI2C_MISO		PA0
#define BI2C_MOSI		PA1
```