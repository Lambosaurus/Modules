# BME280
Support for the BME280 environmental sensor

# Usage

The part is used with minimum oversampling.

```C
I2C_Init(BME280_I2C, I2C_Mode_Fast);
BME280_Init();

while (1)
{
    uint32_t pressure;
    int16_t temp;
    uint8_t hum;
    BME280_Read(&pressure, &temp, &hum);
    ...

    CORE_Idle();
}
```

# Board

The module is dependant on definitions within `Board.h`

The following template can be used for I2C mode

```C
// BME280 interface
#define BME280_I2C          I2C_1

// Configure I2C1
#define I2C1_PINS		    (PB6 | PB7)
#define I2C1_AF			    GPIO_AF1_I2C1
```
