# CC1101
Support for the CC1101 Sub-1GHz radio transciever by Texas Instruments.
This module is highly configurable, and sports an excellent recieve sensitivity.

This part uses an SPI interface.

This driver operates the part with the reciever continously enabled.
The modem is operated with GFSK encoding.

## Usage
Enable/disable of the SPI is the responsibility of the application

The following example listens on channel 0 with address 1, and waits for a message.
In response, a message is send to address 2.

```C
SPI_Init(CC1101_SPI, CC1101_SPI_BITRATE, SPI_Mode_0);
const CC1101_Config_t = {
    .channel = 0,
    .address = 1,
    .power = 10,
}
CC1101_Init(&config);

while (1)
{
    if (CC1101_ReadReady())
    {
        uint8_t bfr[4];
        uint8_t read = CC1101_Read(bfr, sizeof(bfr));
        ...
        uint8_t payload[] = { 0x01, 0x02, 0x04, 0x04 };
        CC1101_Write(2, payload, sizeof(payload));
    }

    CORE_Idle();
}
```

Managing the timing of Writes to avoid conflicting with other devices on the same channel is nessicary to prevent lost messages.


## Board

The module is dependant on  definitions within `Board.h`

The following template can be used. Optional config are commented out.

```C
// CC1101 settings

// Air bitrate in Hz
// Bitrates up to 500Kpbs are supported
#define CC1101_AIR_BAUD			38400

// Frequency of channel 0 in KHz
#define CC1101_FREQ_KHZ			915000

// Deviation between high & low symbols in KHz
#define CC1101_DEV_KHZ			20

// Bandwidth of a recieve channel
// The bandwidth of GFSK can be estimated as CC1101_AIR_BAUD + (CC1101_DEV_KHZ * 2)
#define CC1101_BANDWIDTH_KHZ	80

// The spacing between channels in KHz.
// Channel N will have a frequency of CC1101_FREQ_KHZ + (CC1101_CH_KHZ * N)
// The channel spacing should be greater than the channel bandwidth
#define CC1101_CH_KHZ			100

// Enable forward error correction
// If enabled, the datarate will be half the air bitrate
//#define CC1101_EN_FEC

// Enable manchester encoding
// The datarate will be half the air bitrate
//#define CC1101_EN_MANCHESTER

// Enables a DC filter to improve input sensitivity at a cost of power consumption
// This is ignored at bitrates over 250K.
//#define CC1101_OPTIMISE_SENS

// CC1101 interface
#define CC1101_CS_GPIO      GPIOA
#define CC1101_CS_PIN       GPIO_PIN_1
#define CC1101_GD0_GPIO     GPIOA
#define CC1101_GD0_PIN      GPIO_PIN_0
#define CC1101_MISO_GPIO    GPIOB
#define CC1101_MISO_PIN     GPIO_PIN_4 // Required to sense part ready
#define CC1101_SPI          SPI_1

// Configure SPI1
#define SPI1_GPIO		    GPIOB
#define SPI1_PINS		    (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5)
#define SPI1_AF			    GPIO_AF0_SPI1
#define SPI1_ENABLE
```

## Future work

This part has many features which are unhandled by this driver

* This part can sport excellent low-power characteristics with wake-on-radio
* Alternate encoding schemes, OOK, ASK and more
* More options for preable/sync word config
* Packets longer than 64B can be transmitted in a non packetised mode