#include "CBOR.h"

#include <stdbool.h>
#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */

#define CBOR_MAJOR_BYTE(_m, _n)		(((_m) << 5) | (_n))

/*
 * PRIVATE TYPES
 */

typedef enum {
	CBOR_Major_UnsignedInteger,
	CBOR_Major_NegativeInteger,
	CBOR_Major_ByteString,
	CBOR_Major_TextString,
	CBOR_Major_Array,
	CBOR_Major_Map,
	CBOR_Major_Tag,
	CBOR_Major_Simple,
} CBOR_Major_t;

typedef enum {
	CBOR_Additional_False = 20,
	CBOR_Additional_True = 21,
	CBOR_Additional_Null = 22,
	CBOR_Additional_Undefined = 23,
	CBOR_Additional_1Byte = 24,
	CBOR_Additional_2Byte = 25,
	CBOR_Additional_4Byte = 26,
	CBOR_Additional_8Byte = 27,
	CBOR_Additional_Indefinite = 31,
} CBOR_Additional_t;

/*
 * PRIVATE PROTOTYPES
 */

static inline void CBOR_Put5(CBOR_t * cbor, CBOR_Major_t major, uint8_t n);
static void CBOR_Put8(CBOR_t * cbor, CBOR_Major_t major, uint8_t n);
static void CBOR_Put16(CBOR_t * cbor, CBOR_Major_t major, uint16_t n);
static void CBOR_Put32(CBOR_t * cbor, CBOR_Major_t major, uint32_t n);
static void CBOR_Put64(CBOR_t * cbor, CBOR_Major_t major, uint64_t n);
static void CBOR_PutVar32(CBOR_t * cbor, CBOR_Major_t major, uint32_t n);
static void CBOR_PutVar64(CBOR_t * cbor, CBOR_Major_t major, uint64_t n);
static void CBOR_Puts(CBOR_t * cbor, const uint8_t * data, uint32_t size);
static void CBOR_Putc(CBOR_t * cbor, uint8_t data);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void CBOR_Start(CBOR_t * cbor, uint8_t * bfr, uint32_t size)
{
	cbor->bfr = bfr;
	cbor->head = bfr;
	cbor->end = cbor->head + size;
}

uint32_t CBOR_Finish(CBOR_t * cbor)
{
	if (cbor->head == NULL)
		return 0;
	return cbor->head - cbor->bfr;
}

void CBOR_Array(CBOR_t * cbor, uint32_t count)
{
	if (count == CBOR_INDEFINITE)
		CBOR_Put5(cbor, CBOR_Major_Array, CBOR_Additional_Indefinite);
	else
		CBOR_PutVar32(cbor, CBOR_Major_Array, count);
}

void CBOR_Map(CBOR_t * cbor, uint32_t count)
{
	if (count == CBOR_INDEFINITE)
		CBOR_Put5(cbor, CBOR_Major_Map, CBOR_Additional_Indefinite);
	else
		CBOR_PutVar32(cbor, CBOR_Major_Map, count);
}

void CBOR_Break(CBOR_t * cbor)
{
	CBOR_Putc(cbor, 0xFF);
}

void CBOR_String(CBOR_t * cbor, const char * value)
{
	uint32_t size = strlen(value);
	CBOR_PutVar32(cbor, CBOR_Major_TextString, size);
	CBOR_Puts(cbor, (const uint8_t *)value, size);
}

void CBOR_Bytes(CBOR_t * cbor, const uint8_t * value, uint32_t size)
{
	CBOR_PutVar32(cbor, CBOR_Major_ByteString, size);
	CBOR_Puts(cbor, value, size);
}

void CBOR_Int(CBOR_t * cbor, int32_t value)
{
	if (value >= 0)
		CBOR_PutVar32(cbor, CBOR_Major_UnsignedInteger, value);
	else
		CBOR_PutVar32(cbor, CBOR_Major_NegativeInteger, ~value);
}

void CBOR_Long(CBOR_t * cbor, int64_t value)
{
	if (value >= 0)
		CBOR_PutVar64(cbor, CBOR_Major_UnsignedInteger, value);
	else
		CBOR_PutVar64(cbor, CBOR_Major_NegativeInteger, ~value);
}

void CBOR_Float(CBOR_t * cbor, float value)
{
	// We do not bother doing float compression.
	CBOR_Put32(cbor, CBOR_Major_Simple, *((uint32_t*)&value));
}

void CBOR_Double(CBOR_t * cbor, double value)
{
	// We do not bother doing float compression.
	CBOR_Put64(cbor, CBOR_Major_Simple, *((uint64_t*)&value));
}

void CBOR_Bool(CBOR_t * cbor, bool value)
{
	CBOR_Put5(cbor, CBOR_Major_Simple, value ? CBOR_Additional_True : CBOR_Additional_False);
}

void CBOR_Null(CBOR_t * cbor)
{
	CBOR_Put5(cbor, CBOR_Major_Simple, CBOR_Additional_Null);
}

/*
 * PRIVATE FUNCTIONS
 */

static inline void CBOR_Put5(CBOR_t * cbor, CBOR_Major_t major, uint8_t n)
{
	CBOR_Putc(cbor, CBOR_MAJOR_BYTE(major, n));
}

static void CBOR_Put8(CBOR_t * cbor, CBOR_Major_t major, uint8_t n)
{
	uint8_t data[2] = {
		CBOR_MAJOR_BYTE(major, CBOR_Additional_1Byte),
		n
	};
	CBOR_Puts(cbor, data, sizeof(data));
}

static void CBOR_Put16(CBOR_t * cbor, CBOR_Major_t major, uint16_t n)
{
	uint8_t data[3] = {
		CBOR_MAJOR_BYTE(major, CBOR_Additional_2Byte),
		n >> 8,
		n
	};
	CBOR_Puts(cbor, data, sizeof(data));
}

static void CBOR_Put32(CBOR_t * cbor, CBOR_Major_t major, uint32_t n)
{
	uint8_t data[5] = {
		CBOR_MAJOR_BYTE(major, CBOR_Additional_4Byte),
		n >> 24,
		n >> 16,
		n >> 8,
		n
	};
	CBOR_Puts(cbor, data, sizeof(data));
}

static void CBOR_Put64(CBOR_t * cbor, CBOR_Major_t major, uint64_t n)
{
	uint8_t data[9] = {
		CBOR_MAJOR_BYTE(major, CBOR_Additional_8Byte),
		n >> 56,
		n >> 48,
		n >> 40,
		n >> 32,
		n >> 24,
		n >> 16,
		n >> 8,
		n
	};
	CBOR_Puts(cbor, data, sizeof(data));
}

static void CBOR_PutVar32(CBOR_t * cbor, CBOR_Major_t major, uint32_t n)
{
	if (n < CBOR_Additional_1Byte)
		CBOR_Put5(cbor, major, n);
	else if (n <= 0xFF)
		CBOR_Put8(cbor, major, n);
	else if (n <= 0xFFFF)
		CBOR_Put16(cbor, major, n);
	else
		CBOR_Put32(cbor, major, n);
}

static void CBOR_PutVar64(CBOR_t * cbor, CBOR_Major_t major, uint64_t n)
{
	if (n <= 0xFFFFFFFFULL)
		CBOR_PutVar32(cbor, major, n);
	else
		CBOR_Put64(cbor, major, n);
}

static void CBOR_Puts(CBOR_t * cbor, const uint8_t * data, uint32_t size)
{
	if (cbor->head == NULL)
		return;

	if (cbor->head + size <= cbor->end)
	{
		memcpy(cbor->head, data, size);
		cbor->head += size;
	}
	else
	{
		cbor->head = NULL;
	}
}

static void CBOR_Putc(CBOR_t * cbor, uint8_t data)
{
	if (cbor->head == NULL)
		return;

	if (cbor->head < cbor->end)
		*(cbor->head++) = data;
	else
		cbor->head = NULL;
}

/*
 * INTERRUPT ROUTINES
 */
