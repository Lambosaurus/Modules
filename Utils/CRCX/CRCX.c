
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
                if (crc & 0x01)
                    crc = (crc >> 1) ^ poly;
                else
                    crc >>= 1;
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
                if (crc & 0x80)
                    crc = (crc << 1) ^ poly;
                else
                    crc <<= 1;
            }
      }
      return crc;
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */
