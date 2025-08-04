# COBS

An implementation of Consistent Overhead Byte Stuffing (COBS).

This encodes arbitrary binary packets into zero delimited packets with a very low overhead and simple encode/decode.
The reciever can simply synchronise to packets on the 0x00 delimiter.

Beware that this does not guarantee packet validity.

## Usage

The following shows encoding and decoding.
The `COBS_ENCODE_MAX` and `COBS_DECODE_MAX` are provided for safety - but can be omitted when using sane buffer sizes.

```c
const uint8_t data = { 0x00, 0x01, 0x02, 0x03 }

// Encode data 
uint8_t encoded[COBS_ENCODE_MAX(sizeof_data)];
uint32_t encode_len = COBS_Encode(data, sizeof(data), encoded);
// encoded: { 0x01, 0x04, 0x01, 0x02, 0x03, 0x00 }

uint8_t decoded[16];
if (COBS_DECODE_MAX(encode_len) <= sizeof(decoded))
{
    uint32_t decoded_len = COBS_Decode(encoded, encoded_len, decoded);
    // decoded: { 0x00, 0x01, 0x02, 0x02 }
}
```
