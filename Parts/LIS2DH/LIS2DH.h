#ifndef LIS2DH_H
#define LIS2DH_H

#include "Board.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef enum {
	LIS2_Res_8B,
	LIS2_Res_10B,
	LIS2_Res_12B,
} LIS2_Res_t;

typedef enum {
	LIS2_IntSrc_None 		= 0,
	LIS2_IntSrc_DataReady 	= 1,
	LIS2_IntSrc_Shock 		= 2,
	LIS2_IntSrc_X 			= 0x100,
	LIS2_IntSrc_Y 			= 0x200,
	LIS2_IntSrc_Z 			= 0x400,
	LIS2_IntSrc_XYZ 		= LIS2_IntSrc_X | LIS2_IntSrc_Y | LIS2_IntSrc_Z,
} LIS2_IntSrc_t;

typedef struct {
	LIS2_Res_t resolution;
	LIS2_IntSrc_t int_src;
	uint8_t scale_g;
	uint16_t frequency;
	uint16_t threshold;
} LIS2_Config_t;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} LIS2_Accel_t;

/*
 * PUBLIC FUNCTIONS
 */

bool LIS2_Init(const LIS2_Config_t * cfg);
void LIS2_Deinit(void);
void LIS2_Read(LIS2_Accel_t * acc);
bool LIS2_IsIntSet(void);

#endif //LIS2DH_H
