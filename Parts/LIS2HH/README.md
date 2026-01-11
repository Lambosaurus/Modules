# LIS2HH
Support for the LIS2HH 3 axis accelerometer by ST Microelectronics
This uses the SPI or I2C interface.

# Usage

This part is highly configurable - only simple configurations are supported by this driver.

Read the datasheet for more info on the available config. Note that not all datarates are available for all resolutions.

The following config demonstrates a 100Hz sampling regeim

```C
SPI_Init(LIS2HH_SPI, LIS2HH_SPI_BITRATE, SPI_Mode_0);
LIS2HH_Init(2, 100, true); // 2G mode, 100Hz, HP mode
LIS2HH_EnableDataInt();

while (1)
{
    if (LIS2HH_IsIntSet())
    {
        LIS2HH_Accel_t accel;
        LIS2HH_Read(&accel);
        ...
    }

    CORE_Idle();
}
```

# Board

The module is dependant on definitions within `Board.h`

The following template can be used for SPI mode

```C
// LIS2HH interface
#define LIS2HH_SPI            SPI_1
#define LIS2HH_CS_PIN         PA0
#define LIS2HH_INT_PIN        PA1
#define GPIO_IRQ1_ENABLE

// Configure SPI1
#define SPI1_PINS		    (PB3 | PB4 | PB5)
#define SPI1_AF			    GPIO_AF0_SPI1
#define SPI1_ENABLE
```

The following template can be used for I2C mode

```C
// LIS2HH interface
#define LIS2HH_I2C            I2C_1
#define LIS2HH_INT_GPIO       PA1
#define GPIO_IRQ1_ENABLE

// Configure I2C1
#define I2C1_PINS		    (PB6 | PB7)
#define I2C1_AF			    GPIO_AF1_I2C1
```
