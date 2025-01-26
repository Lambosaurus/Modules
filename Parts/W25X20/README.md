# W25X20
Support for the Winbond W25X20CLUXIG Flash memory
This uses an SPI inteface.

This is probably compatible with a wide range of other flash memory, as the command set is mostly standard.

## Usage
Note that the SPI enable/disable is the responsibility of the user.

The following example initialises the part, and reads out some data.
Reads have no alignment requirements.

```C
SPI_Init(W25X20_SPI, 16000000, SPI_Mode_0);
if (W25X20_Init())
{
    // Read the first 256 bytes of memory.
    uin8_t data[256];
    W25X20_Read(0, data, sizeof(data))
    ...
}
```

## Writing

Writes must be managed.
 * Writes may not cross 256 byte page boundaries
 * Erases must be in 4K sectors

The following example writes a 4K block of data.

```C
SPI_Init(W25X20_SPI, 16000000, SPI_Mode_0);
if (W25X20_Init())
{
    W25X20_Erase(0, 4096);
    
    uint32_t addr = 0;
    for (uint32_t addr = 0; addr < 4096; addr += 256)
    {
        uint8_t page[256];
        // User function to get the next chunk.
        User_GetData(addr, data, sizeof(data));

        W25X20_Write(addr, data, sizeof(data));
    }
}
```

Arbitrarially large chunks can be erased at once, and the driver will do this as efficiently as possible.

For example: `W25X20_Erase(4 * 1024, 128 * 1024);` will use the following sequence to satisfy the chunk and alignment requirements:
1. 7x 4K erase commands
2. 1x 32K erase command
3. 1x 64K erase command
4. 1x 4K erase command

## Power management

The W25X20 needs to be put into powerdown mode to minimize power consumption.
Use `W25X20_Deinit` to achieve this. Note that this does not deinit the CS pin by default.


## Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
// W25X20 interface
#define W25X20_SPI			SPI_1
#define W25X20_CS_PIN		PA0

// Configure SPI1
#define SPI1_PINS		    (PA5 | PA6 | PA7)
#define SPI1_AF				GPIO_AF0_SPI1
```