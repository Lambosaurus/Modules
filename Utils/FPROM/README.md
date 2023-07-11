# FPROM
This is an EEPROM replacement for flash only devices.
This uses the last flash page to provide an equivilent API to the EEPROM module

## Usage

The following example reads a structure, modifies it, then writes it back.

```C

static struct {
    uint32_t a;
    uint32_t b;
} data;

FPROM_Read(0, &data, sizeof(data));
...
data.a = 10;
FPROM_Write(0, &data, sizeof(data));

```

## Page size

The size of the FPROM page is defined as `FPROM_PAGE_SIZE`.
This may not exceed `FLASH_PAGE_SIZE`, and must be able to contain the stored data.

## Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
// Set the size of the FPROM page
#define FPROM_PAGE_SIZE 256
```