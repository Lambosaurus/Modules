#include "NMEA.h"
#include <stdlib.h>


#ifndef NMEA_MESSAGE_MAX
#define NMEA_MESSAGE_MAX	96
#endif

#define NMEA_MSG_NOSTART	0xFF

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool NMEA_Decode(NMEA_Message_t * msg, char * str);

#ifdef NMEA_VALIDATE_CHECKSUM
static bool NMEA_ValidateChecksum(NMEA_Message_t * msg, char * str);
#endif

#ifdef NMEA_DECODE_RMC
static bool NMEA_DecodeRMC(NMEA_Message_t * msg, char * str);
#endif
#ifdef NMEA_DECODE_GGA
static bool NMEA_DecodeGGA(NMEA_Message_t * msg, char * str);
#endif
#ifdef NMEA_DECODE_GLL
static bool NMEA_DecodeGLL(NMEA_Message_t * msg, char * str);
#endif
#ifdef NMEA_DECODE_ZDA
static bool NMEA_DecodeZDA(NMEA_Message_t * msg, char * str);
#endif


static const char * NMEA_NextField(char ** str);
static char NMEA_NextChar(char ** str);
static int32_t NMEA_NextInt(char ** str);
static int32_t NMEA_NextNumber(char ** str, uint8_t precision);
static void NMEA_NextDate(char ** str, uint8_t * year, uint8_t * month, uint8_t * day);
static void NMEA_NextTime(char ** str, uint8_t * hour, uint8_t * minute, uint8_t * second, uint16_t * millisecond);
static int32_t NMEA_NextLatitude(char ** str);
static int32_t NMEA_NextLongitude(char ** str);

static uint32_t NMEA_Atou(const char ** str, uint8_t limit);
static int32_t NMEA_Atofp(const char ** str, uint8_t precision);
static uint32_t NMEA_Power10(uint32_t value, uint8_t power);

/*
 * PRIVATE VARIABLES
 */

static struct {
	NMEA_Callback_t callback;
	char bfr[NMEA_MESSAGE_MAX];
	uint8_t count;
} gNMEA;

/*
 * PUBLIC FUNCTIONS
 */

void NMEA_Init(NMEA_Callback_t callback)
{
	gNMEA.callback = callback;
	gNMEA.count = NMEA_MSG_NOSTART;
}

void NMEA_Parse(uint8_t * bfr, uint32_t size)
{
	while (size--)
	{
		char c = (*bfr++);

		if (c == '$')
		{
			gNMEA.count = 0;
		}
		else if (c == '\n' && gNMEA.count != NMEA_MSG_NOSTART)
		{
			uint8_t len = gNMEA.count;
			gNMEA.count = NMEA_MSG_NOSTART;

			if (gNMEA.count > 8)
			{
				// Strip CR and null terminate.
				if (gNMEA.bfr[len-1] == '\r')
					len--;
				gNMEA.bfr[len] = 0;

				NMEA_Message_t msg;
				if (NMEA_Decode(&msg, gNMEA.bfr))
				{
					gNMEA.callback(&msg);
				}
			}
		}
		else if (gNMEA.count < sizeof(gNMEA.bfr) - 1)
		{
			gNMEA.bfr[gNMEA.count++] = c;
		}
	}
}

/*
 * PRIVATE FUNCTIONS: DECODERS
 */

static bool NMEA_Decode(NMEA_Message_t * msg, char * str)
{
	// We get a NMEA message, but not the '$' or line ending.
	// The string is null terminated, so we can rely on that.

#ifdef NMEA_VALIDATE_CHECKSUM
	if (!NMEA_ValidateChecksum(msg, str))
		return false;
#endif

#ifdef NMEA_DECODE_RMC
	if (NMEA_DecodeRMC(msg, str))
		return true;
#endif
#ifdef NMEA_DECODE_GGA
	if (NMEA_DecodeGGA(msg, str))
		return true;
#endif
#ifdef NMEA_DECODE_GLL
	if (NMEA_DecodeGLL(msg, str))
		return true;
#endif
#ifdef NMEA_DECODE_ZDA
	if (NMEA_DecodeZDA(msg, str))
		return true;
#endif
	return false;
}


#ifdef NMEA_VALIDATE_CHECKSUM
static int8_t NMEA_ParseHex(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	c &= ~('a' ^ 'A');
	if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}
	return -1;
}

static bool NMEA_ValidateChecksum(NMEA_Message_t * msg, char * str)
{
	uint8_t checksum = 0;
	while (*str != 0)
	{
		char c = *str++;
		if (c == '*')
		{
			// This is safe because the string is null terminated.
			return 	((int8_t)(checksum >> 4)   == NMEA_ParseHex(*str++))
				&&  ((int8_t)(checksum & 0x0F) == NMEA_ParseHex(*str));

		}
		checksum ^= c;
	}
	return false;
}
#endif //NMEA_VALIDATE_CHECKSUM


#ifdef NMEA_DECODE_RMC
static bool NMEA_DecodeRMC(NMEA_Message_t * msg, char * str)
{
	// $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A

	if (strncmp(str+2, "RMC", 3) != 0)
		return false;
	str += 6;

	msg->type = NMEA_Type_RMC;
	NMEA_NextTime(&str, &msg->rmc.hour, &msg->rmc.minute, &msg->rmc.second, &msg->rmc.millisecond);
	msg->rmc.valid = NMEA_NextChar(&str) == 'A';
	msg->rmc.latitude = NMEA_NextLatitude(&str);
	msg->rmc.longitude = NMEA_NextLongitude(&str);
	msg->rmc.speed = (NMEA_NextNumber(&str, 2) * 1317) >> 8; // into mm/s
	msg->rmc.heading = NMEA_NextNumber(&str, 1);
	NMEA_NextDate(&str, &msg->rmc.year, &msg->rmc.month, &msg->rmc.day);

	return true;
}
#endif // NMEA_DECODE_RMC


#ifdef NMEA_DECODE_GGA
static bool NMEA_DecodeGGA(NMEA_Message_t * msg, char * str)
{
	if (strncmp(str+2, "GGA", 3) != 0)
		return false;
	str += 6;

	msg->type = NMEA_Type_GGA;
	NMEA_NextTime(&str, &msg->gga.hour, &msg->gga.minute, &msg->gga.second, &msg->gga.millisecond);
	msg->gga.latitude = NMEA_NextLatitude(&str);
	msg->gga.longitude = NMEA_NextLongitude(&str);
	msg->gga.quality = NMEA_NextInt(&str);
	msg->gga.satellites = NMEA_NextInt(&str);
	msg->gga.hdop = NMEA_NextNumber(&str, 2);
	msg->gga.altitude = NMEA_NextNumber(&str, 3);

	return true;
}
#endif //NMEA_DECODE_GGA

#ifdef NMEA_DECODE_GLL
static bool NMEA_DecodeGLL(NMEA_Message_t * msg, char * str)
{
	if (strncmp(str+2, "GLL", 3) != 0)
		return false;
	str += 6;

	msg->type = NMEA_Type_GLL;
	msg->gll.latitude = NMEA_NextLatitude(&str);
	msg->gll.longitude = NMEA_NextLongitude(&str);
	NMEA_NextTime(&str, &msg->gll.hour, &msg->gll.minute, &msg->gll.second, &msg->gll.millisecond);
	msg->gll.valid = NMEA_NextChar(&str) == 'A';

	return true;
}
#endif //NMEA_DECODE_GLL

#ifdef NMEA_DECODE_ZDA
static bool NMEA_DecodeZDA(NMEA_Message_t * msg, char * str)
{
	if (strncmp(str+2, "ZDA", 3) != 0)
		return false;
	str += 6;

	msg->type = NMEA_Type_ZDA;
	NMEA_NextTime(&str, &msg->zda.hour, &msg->zda.minute, &msg->zda.second, &msg->zda.millisecond);
	msg->zda.day = NMEA_NextInt(&str);
	msg->zda.month = NMEA_NextInt(&str);
	msg->zda.year = NMEA_NextInt(&str);
	msg->zda.timezone_hours = NMEA_NextInt(&str);
	msg->zda.timezone_minutes = NMEA_NextInt(&str);

	return true;
}
#endif //NMEA_DECODE_ZDA

/*
 * PRIVATE FUNCTIONS: FIELD UTILITIES
 */


static const char * NMEA_NextField(char ** str)
{
	char * token = *str;
	char * head = token;

	// Find a delimiter.
	// TODO: We could possibly avoid checking for the '*' token.
    while (*head && *head != ',' && *head != '*')
        head++;

    // Null terminate the token, then walk past
    if (*head != 0)
    	*head++ = 0;

    *str = head;
    return token;
}

__attribute__((unused))
static char NMEA_NextChar(char ** str)
{
	const char * token = NMEA_NextField(str);
	return *token;
}

__attribute__((unused))
static int32_t NMEA_NextInt(char ** str)
{
	const char * token = NMEA_NextField(str);
	if (*token == 0) return 0;
	return NMEA_Atou(&token, 10);
}

__attribute__((unused))
static int32_t NMEA_NextNumber(char ** str, uint8_t precision)
{
	const char * token = NMEA_NextField(str);
    if (*token == 0) return 0;
    return NMEA_Atofp(&token, precision);
}

__attribute__((unused))
static void NMEA_NextDate(char ** str, uint8_t * year, uint8_t * month, uint8_t * day)
{
	const char * token = NMEA_NextField(str);
	*day = NMEA_Atou(&token, 2);
	*month = NMEA_Atou(&token, 2);
	*year = NMEA_Atou(&token, 2);
}

__attribute__((unused))
static void NMEA_NextTime(char ** str, uint8_t * hour, uint8_t * minute, uint8_t * second, uint16_t * millisecond)
{
	const char * token = NMEA_NextField(str);
	*hour = NMEA_Atou(&token, 2);
	*minute = NMEA_Atou(&token, 2);
	*second = NMEA_Atou(&token, 2);
	if (*token == '.')
	{
		token++;
		*millisecond = NMEA_Atou(&token, 3);
	}
	else
	{
		*millisecond = 0;
	}
}

__attribute__((unused))
static int32_t NMEA_NextCoordinate(char ** str, uint8_t digits, char negative)
{
	const char * token = NMEA_NextField(str);
	uint32_t degrees = NMEA_Atou(&token, digits);
	uint32_t minutes = NMEA_Atofp(&token, 3);
	uint32_t coord = (degrees * 1000000) + (minutes * 1000 / 60);

	char hemisphere = NMEA_NextChar(str);
	return hemisphere == negative ? -coord : coord;
}

__attribute__((unused))
static int32_t NMEA_NextLatitude(char ** str)
{
	return NMEA_NextCoordinate(str, 2, 'S');
}

__attribute__((unused))
static int32_t NMEA_NextLongitude(char ** str)
{
	return NMEA_NextCoordinate(str, 3, 'W');
}

/*
 * PRIVATE FUNCTIONS: FIELD UTILITIES
 */

static uint32_t NMEA_Atou(const char ** str, uint8_t limit)
{
	const char * head = *str;

	uint32_t value = 0;
	while (limit && *head >= '0' && *head <= '9')
	{
		limit--;
		value *= 10;
		value += (*head++) - '0';
	}

	*str = head;
	return value;
}

static int32_t NMEA_Atofp(const char ** str, uint8_t precision)
{
	const char * head = *str;

    bool negative = false;
    if (*head == '-') { negative = true; head++; }

    int32_t value = NMEA_Atou(&head, 10);
    value = NMEA_Power10(value, precision);

    if (*head == '.')
    {
    	head++;
    	const char * start = head;
    	int32_t low = NMEA_Atou(&head, precision);
    	uint8_t digits = head - start;
    	low = NMEA_Power10(low, precision - digits);
    	value += low;
    }

    *str = head;
    return negative ? -value : value;
}

static uint32_t NMEA_Power10(uint32_t value, uint8_t power)
{
	while (power--)
		value *= 10;
	return value;
}


