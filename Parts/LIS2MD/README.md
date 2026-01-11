# LIS2MD
Support for the LIS2DT 3 axis magnetometer by ST Microelectronics
This uses the SPI or I2C interface.

# Usage

This shows basic usage.

```C
SPI_Init(LIS2MD_SPI, LIS2MD_SPI_BITRATE, SPI_Mode_0);
LIS2MD_Init(10, false); // 10Hz LP mode

while (1)
{
    // Data available at 10Hz
    LIS2MD_Mag_t mag;
    LIS2MD_Read(&mag);

    CORE_Delay(100);
}
```

Interrupts are not yet supported


# Board

The module is dependant on definitions within `Board.h`

The following template can be used for SPI mode

```C
// LIS2MD interface
#define LIS2MD_SPI            SPI_1
#define LIS2MD_CS_PIN         PA0

// Configure SPI1
#define SPI1_PINS		    (PB3 | PB4 | PB5)
#define SPI1_AF			    GPIO_AF0_SPI1
#define SPI1_ENABLE
```

The following template can be used for I2C mode

```C
// LIS2DT interface
#define LIS2MD_I2C            I2C_1

// Configure I2C1
#define I2C1_PINS		    (PB6 | PB7)
#define I2C1_AF			    GPIO_AF1_I2C1
```
