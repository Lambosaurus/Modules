#ifndef NMEA_H
#define NMEA_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef enum {
	NMEA_Type_None = 0,
	NMEA_Type_RMC,
	NMEA_Type_GGA,
	NMEA_Type_GLL,
	NMEA_Type_ZDA,
} NMEA_Type_t;

typedef struct {
	int32_t latitude;     // in micro-deg
	int32_t longitude;    // in micro-deg
	uint32_t speed;		  // in mm/s
	uint16_t heading; 	  // in deci-deg
	bool valid;
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint16_t millisecond;
} NMEA_RMC_t;

typedef struct {
	int32_t latitude;	// in micro-deg
	int32_t longitude;	// in micro-deg
	int32_t altitude;	// in mm
	uint16_t hdop;		// 0.01 units
	uint8_t quality;	// 0 is invalid
	uint8_t satellites;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint16_t millisecond;
} NMEA_GGA_t;

typedef struct {
	int32_t latitude;	// in micro-deg
	int32_t longitude;	// in micro-deg
	bool valid;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint16_t millisecond;
} NMEA_GLL_t;

typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint16_t millisecond;
	uint8_t timezone_hours;
	uint8_t timezone_minutes;
} NMEA_ZDA_t;

typedef struct {
	NMEA_Type_t type;
	union {
#ifdef NMEA_DECODE_RMC
		NMEA_RMC_t rmc;
#endif
#ifdef NMEA_DECODE_GGA
		NMEA_GGA_t gga;
#endif
#ifdef NMEA_DECODE_GLL
		NMEA_GLL_t gll;
#endif
#ifdef NMEA_DECODE_ZDA
		NMEA_ZDA_t zda;
#endif
	};
} NMEA_Message_t;


typedef void(*NMEA_Callback_t)(const NMEA_Message_t * msg);

/*
 * PUBLIC FUNCTIONS
 */

void NMEA_Init(NMEA_Callback_t callback);
void NMEA_Parse(uint8_t * bfr, uint32_t size);

/*
 * EXTERN DECLARATIONS
 */


#endif // NMEA_H
