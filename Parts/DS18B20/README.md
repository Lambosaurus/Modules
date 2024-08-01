# DS18B20
DS18B20 one wire temperature sensors.

See the [OneWire](../../Utils/OneWire/README.md) module for information on identifying one wire devices.

## Usage

The following shows a temperature read.

```C
D1W_Init(PA0);

// NULL address can be used if only one device is on the bus.
int32_t temp;
if (DS18B20_ReadTemperature(NULL, &temp))
{
    // Temperature is expressed in millidegrees
}

D1W_Deinit();
```

The device is assumed to be in 12bit mode.

## Dependencies

This is dependent on the [OneWire](../../Utils/OneWire) module.