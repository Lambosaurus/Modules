# SX1276
Support for the Semtech SX1276 LoRa transceiver.

Only Lora mode is supported by this driver.

## Usage

Below is an example of repeated transmits.

```C
SPI_Init(SX1276_SPI, 8000000, SPI_Mode_0);

SX1276_Init();

SX1276_Configure(
    915000000,                 // Frequency (Hz)
    SX1276_Bandwidth_125kHz,
    SX1276_SpreadingFactor_7,
    SX1276_CodingRate_4_5,
    SX1276_SyncWord_Private,
    8,                         // Preamble length
    SX1276_Flag_CrcEnable,     // Flags
    15                         // TX power (dBm)
);

while (1)
{
    SX1276_Update();

    // Wait for return to idle before next transmit.
    if (SX1276_GetState() == SX1276_State_Idle)
    {
        const uint8_t msg[] = { 0x00, 0x01, 0x02, 0x03 };
        SX1276_Send(msg, sizeof(msg));
    }

    CORE_Idle();
}
```

The following demonstrates a continous receive.

```C
SX1276_StartReceive();

while (1)
{
    SX1276_Update();

    uint8_t buf[32];
    int16_t rssi;
    uint32_t received = SX1276_Receive(buf, sizeof(buf), &rssi);

    if (received)
    {
        ...
    }

    CORE_Idle();
}
```

`SX1276_Stop()` can be used to halt current TX or RX state.

## Configuration

The RX may use the LF or HF ports.
Either `SX1276_USE_LF_PORT` or `SX1276_USE_HF_PORT` must be defined.

The TX may use either the RFO or PA_BOOST ports.
Either `SX1276_USE_RFO` or `SX1276_USE_PA_BOOST` must be defined.

## Board

The module is dependant on  definitions within `Board.h`
The following template can be used.

```C
// Configure SPI1
#define SPI1_PINS           (PB3 | PB4 | PB5)
#define SPI1_AF             GPIO_AF0_SPI1

// IRQ for DIO0
#define GPIO_IRQ1_ENABLE

// SX1276 config
#define SX1276_SPI              SPI_1
#define SX1276_CS_PIN           PA0
#define SX1276_DIO0_PIN         PA1
//#define SX1276_RST_PIN        PC15 // Optional reset pin
//#define SX1276_SWITCH_PIN     PB1  // Optional RF switch pin
//#define SX1276_SWITCH_TXPOL   GPIO_PIN_RESET
//#define SX1276_OSC_TCXO       // Set if OSC is TXCO instead of XTAL

//#define SX1276_USE_PA_BOOST
//#define SX1276_USE_RFO
//#define SX1276_USE_LF_PORT
//#define SX1276_USE_HF_PORT
```