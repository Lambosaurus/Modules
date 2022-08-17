# RC4
This implements the RC4 cypher.

This is a very compact stream cypher.

Its advantages are:
 * Very simple code
 * Output data is same size as input data
 * Data does not need to conform to any block sizes

Its disadvantages are:
 * As an XOR stream cypher, reuse of the key leaks the XOR stream.
 * This cypher is no longer considered secure

## Usage

The following example encodes a messages using RC4

```C
const uint8_t KEY = { 0xC4, 0xC2, 0xE6, 0xC3, 0x48, 0x73, 0xA7, 0x33 }

// Initialise the cypher
RC4_t rc4;
RC4_Init(&rc4, KEY, sizeof(KEY));

// Encode the plaintext
uint8_t plaintext[4] = {0x01, 0x02, 0x03, 0x04}
uint8_t encoded[4];
RC4_Encrypt(&rc4, plaintext, encoded, sizeof(plaintext));


// Re-initialize the cypher to reset the cyper position
RC4_t rc4;
RC4_Init(&rc4, KEY, sizeof(KEY));

// Decode the plaintext. Note that Encryption and Decryption are the same operation.
uint8_t decoded[4];
RC4_Encrypt(&rc4, encoded, decoded, sizeof(encoded));
```

Note that text can be encoded in place
```C
uint8_t text[4] = {0x01, 0x02, 0x03, 0x04}
RC4_Encrypt(&rc4, text, text, sizeof(text));
// text is now encrypted.
```

## Key protection

Most importantly: avoid re-using keys. This causes the keystream to be leaked.
This can be done simply using a random nonce

```C
const uint8_t PSK = { 0xC4, 0xC2, 0xE6, 0xC3, 0x48, 0x73, 0xA7, 0x33 }

// A source of random is required
uint8_t nonce[8];
RNG_ReadBytes(nonce, sizeof(nonce));

// Create a composite key with nonce + PSK
uint8_t key[sizeof(nonce) + sizeof(PSK)];
memcpy(key, nonce, sizeof(nonce));
memcpy(key + sizeof(nonce), key, sizeof(PSK));

// Initialise the cypher
RC4_t rc4;
RC4_Init(&rc4, key, sizeof(key));

uint8_t plaintext[4] = {0x01, 0x02, 0x03, 0x04}

uint8_t encoded[ sizeof(nonce) + sizeof(plaintext) ];

// The destination is prefixed with the nonce - so that the decoder and recover it and create the same key
memcpy(encoded, nonce, sizeof(nonce));

// The encrypted body follows the nonce.
RC4_Encrypt(&rc4, encoded + sizeof(nonce), plaintext, sizeof(plaintext));
```

Hashing the nonce and PSK is significantly more secure if this is available.

