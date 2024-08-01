# OneWire
Dallas One Wire implementation.

Some notes:
 - Overdrive mode is not implemented.
 - The pin is used in open drain mode. This could optionally be output high for TX bits.

## Usage

All read/write sequences must start with a `D1W_SelectRom` call.

The following example reads data from a single device in broadcast mode.

```C
D1W_Init(PA0);

// ROM = NULL broadcasts to all devices on the bus
if (D1W_SelectRom(NULL))
{
    D1W_WriteByte(0xBE);
    uint8__t rx[9];
    D1W_Read(rx, sizeof(rx));
    ...
}

D1W_Deinit();
```

## ROM commands

The ROM is an 8 byte unique identifier for the one wire device. This ROM must be known to address a specific device.

`D1W_SelectRom` can be used with a ROM to select a known device, or NULL to broadcast to all devices.

`D1W_ReadRom` can be used when only one device exists on the bus, otherwise the `D1W_SearchRom` must be used to enuemrate the devices.

## ROM searching

`D1W_SearchRom` can be used to enumerate the devices on the bus. It will return true for each found device.

Note that the initial ROM is the start point of the search, so should be initialized to zero.

```C
D1W_Init(PA0);

// Enumerate through all ROMS
uint8_t rom[8] = {0};
while (D1W_SearchRom(rom))
{
    // We have a valid ROM. We can store this or use it.
    D1W_SelectRom(rom);
    uint8_t data = D1W_ReadByte();
}

D1W_Deinit();
```

## CRC

The one wire CRC8 is implemented as `D1W_CRC`.

```C
D1W_Init(PA0);

uint8_t rom[8];
if (D1W_ReadRom(rom))
{
    // The last ROM byte is a CRC
    bool rom_valid = D1W_CRC(rom, 7) == rom[7];
}

D1W_Deinit();
```

## Dependencies

This is dependent on the [CRCX](../../Utils/CRCX) module for its CRC8 implementation.