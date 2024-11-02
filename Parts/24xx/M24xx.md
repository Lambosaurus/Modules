# 24LC04
This implements support for the Microchip 24xx EEPROM's

| Series | Part number | Size   | Page  |
| ------ | ----------- | ------ | ----- |
| 0      | 24xx00x     | 16 B   | -     |
| 1      | 24xx01x     | 128 B  | 8 B   |
| 2      | 24xx02x     | 256 B  | 8 B   | 
| 4      | 24xx04x     | 512 B  | 16 B  | 
| 8      | 24xx08x     | 1 KB   | 16 B  | 
| 16     | 24xx16x     | 2 KB   | 16 B  |
| 32     | 24xx32x     | 4 KB   | 32 B  | 
| 64     | 24xx64x     | 8 KB   | 32 B  | 
| 128    | 24xx128x    | 16 KB  | 64 B  |
| 256    | 24xx256x    | 32 KB  | 64 B  | 
| 512    | 24xx512x    | 64 KB  | 128 B |
| 1024   | 24xx1025x   | 128 KB | 128 B | 

## Usage

This module does not take responsibility for init/deinit of the I2C peripheral.

```C
I2C_Init(M24XX_I2C, I2C_Mode_Fast);
M24xx_Init();

while (1)
{
    // Write data to the EEPROM
    uint8_t tx[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    M24xx_Write(0, tx, sizeof(tx))

    ...

    // Read data from the EEPROM
    uint8_t rx[8];
    M24xx_Read(0, rx, sizeof(rx))
}
```

`M24xx_Write` handles pagination of the writes, so can be called with an arbitrary position and length.

## Multiple devices

For devices with functional address pins, `A[2:0]` can be used to organize the memory into a consecutive block.

For example, up to 8x `24AA014` can be used, and will be used as consecutive 128 byte blocks.

> `M24xx_Read` may not currently support reads across a device boundary.

## Board

The module is dependant on  definitions within `Board.h`
The following template can be used.

```C
// Configure I2C1
#define I2C1_PINS		    (PB6 | PB7)
#define I2C1_AF			    GPIO_AF1_I2C1

// M24XX config
#define M24XX_I2C	        I2C_1
#define M24XX_SERIES        1
```