# LIS2
Support for the LIS2DT 3 axis accelerometer by ST Microelectronics
This uses the SPI or I2C interface.

## Usage

This part is highly configurable - only simple configurations are supported by this driver.

Read the datasheet for more info on the available config. Note that not all datarates are available for all resolutions.

The following config demonstrates a 100Hz sampling regeim

```C
const LIS2_Config_t config = {
    .resolution = LIS2_Res_12B,
    .scale_g = 2,
    .frequency = 100,
    .int_src = LIS2_IntSrc_DataReady,
    .threshold = 0,
}

SPI_Init(LIS2_SPI, LIS2_SPI_BITRATE, SPI_Mode_0);
LIS2_Init(&config);

while (1)
{
    if (LIS2_IsIntSet())
    {
        LIS2_Accel_t accel;
        LIS2_Read(&accel);
        ...
    }

    CORE_Idle();
}
```

An alternative configuration could be used for impact detection

```C
const LIS2_Config_t config = {
    .resolution = LIS2_Res_12B,
    .scale_g = 4,
    .frequency = 100,
    .int_src = LIS2_IntSrc_Shock,
    .threshold = 2000,
}
```

## Board

The module is dependant on definitions within `Board.h`

The following template can be used for SPI mode

```C
// LIS2DH interface
#define LIS2_SPI            SPI_1
#define LIS2_CS_GPIO        GPIOA
#define LIS2_CS_PIN         GPIO_PIN_0
#define LIS2_INT_GPIO       GPIOA
#define LIS2_INT_PIN        GPIO_PIN_1
#define GPIO_IRQ1_ENABLE

// Configure SPI1
#define SPI1_GPIO		    GPIOB
#define SPI1_PINS		    (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5)
#define SPI1_AF			    GPIO_AF0_SPI1
#define SPI1_ENABLE
```

The following template can be used for I2C mode

```C
// LIS2DH interface
#define LIS2_SPI            SPI_1
#define LIS2_INT_GPIO       GPIOA
#define LIS2_INT_PIN        GPIO_PIN_1
#define GPIO_IRQ1_ENABLE

// Configure I2C1
#define I2C1_GPIO		    GPIOB
#define I2C1_PINS		    (GPIO_PIN_6 | GPIO_PIN_7)
#define I2C1_AF			    GPIO_AF1_I2C1
```
