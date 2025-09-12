# NMEA

A lightweight and configurable NMEA parser.


# Usage


# Board

The module is dependent on  definitions within `Board.h`

The following template can be used.

```C
// NMEA
#define NMEA_DECODE_RMC
//#define NMEA_DECODE_GGA
//#define NMEA_DECODE_GLL
//#define NMEA_DECODE_ZDA
//#define NMEA_MESSAGE_MAX            82
#define NMEA_VALIDATE_CHECKSUM
```

# TODO:

1. Terminate message on '*' if not validating checksum?
2. Adjust default packet limit to 82.
3. Discard packets if length exceeded.
4. Add an error if packet limit over 255?
5. Timestamp to millis?