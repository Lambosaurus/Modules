# LIS3
Support for the LIS3DH 3 axis accelerometer by ST Microelectronics  
This uses the SPI or I2C interface.

## Usage

This part is highly configurable - only simple configurations are supported by this driver.

Read the datasheet for more info on the available config. Note that not all datarates are available for all resolutions.

The following config demonstrates a 100Hz sampling regeim

```C
const LIS3_Config_t config = {
    .resolution = LIS3_Res_12B,
    .scale_g = 2,
    .frequency = 100,
    .int_src = LIS3_IntSrc_DataReady,
    .threshold = 0,
};

SPI_Init(LIS3_SPI, LIS3_SPI_BITRATE, SPI_Mode_0);
LIS3_Init(&config);

while (1)
{
    if (LIS3_IsIntSet())
    {
        LIS3_Accel_t accel;
        LIS3_Read(&accel);
        ...
    }

    CORE_Idle();
}
```

An alternative configuration could be used for impact detection

```C
const LIS3_Config_t config = {
    .resolution = LIS3_Res_12B,
    .scale_g = 4,
    .frequency = 100,
    .int_src = LIS3_IntSrc_Shock,
    .threshold = 2000,
};
```

## Board

The module is dependant on definitions within `Board.h`

The following template can be used for SPI mode

```C
// LIS3DH interface
#define LIS3_SPI            SPI_1
#define LIS3_CS_PIN         PA0
#define LIS3_INT_PIN        PA1
#define GPIO_IRQ1_ENABLE

// Configure SPI1
#define SPI1_PINS		    (PB3 | PB4 | PB5)
#define SPI1_AF			    GPIO_AF0_SPI1
#define SPI1_ENABLE
```

The following template can be used for I2C mode

```C
// LIS3DH interface
#define LIS3_I2C            I2C_1
#define LIS3_INT_PIN        PA1
#define GPIO_IRQ1_ENABLE

// Configure I2C1
#define I2C1_PINS		    (PB6 | PB7)
#define I2C1_AF			    GPIO_AF1_I2C1
```
