
#include "CRCX.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

uint8_t CRC8(uint8_t init, uint8_t poly, const uint8_t * data, uint32_t size)
{
      uint8_t crc = init;
      while (size--)
      {
            crc ^= *data++;
            for (uint32_t i = 0; i < 8; i++)
            {
                if (crc & 0x80)
                    crc = (crc << 1) ^ poly;
                else
                    crc <<= 1;
            }
      }
      return crc;
}

uint8_t CRC8R(uint8_t init, uint8_t poly, const uint8_t * data, uint32_t size)
{
      uint8_t crc = init;
      while (size--)
      {
            crc ^= *data++;
            for (uint32_t i = 0; i < 8; i++)
            {
                if (crc & 0x01)
                    crc = (crc >> 1) ^ poly;
                else
                    crc >>= 1;
            }
      }
      return crc;
}

uint16_t CRC16(uint16_t init, uint16_t poly, const uint8_t * data, size_t size)
{
	uint16_t crc = init;
	while (size--)
	{
		crc ^= *data++ << 8;
		for (uint32_t i = 0; i < 8; i++)
		{
			if (crc & 0x8000)
				crc = (crc << 1) ^ poly;
			else
				crc = crc << 1;
		}
	}
	return crc;
}

uint16_t CRC16R(uint16_t init, uint16_t poly, const uint8_t * data, size_t size)
{
	uint16_t crc = init;
	while (size--)
	{
		crc ^= *data++;
		for (uint32_t i = 0; i < 8; i++)
		{
			if (crc & 0x0001)
				crc = (crc >> 1) ^ poly;
			else
				crc = crc >> 1;
		}
	}
	return crc;
}


uint16_t CRC16_CCITT(uint16_t init, const uint8_t * data, size_t size)
{
	uint16_t crc = init;
	while (size--)
	{
		uint8_t e = crc ^ *data++;
		uint16_t f = (uint8_t)(e ^ (e << 4));
		crc = (crc >> 8) ^ (f << 8) ^ (f << 3) ^ (f >> 4);
	}
	return crc;
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */
