#ifndef LIS3DH_H
#define LIS3DH_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#define LIS3_SPI_BITRATE   10000000 // 10MHz

/*
 * PUBLIC TYPES
 */

typedef enum {
    LIS3_Res_12B,
    LIS3_Res_14B,
} LIS3_Res_t;

typedef enum {
    // This will set up for triggered conversion mode
    LIS3_IntSrc_None       = 0,

    // This will interrupt on every conversion
    LIS3_IntSrc_DataReady  = 1,

    // This configures the threshold as a delta, not an absolute
    LIS3_IntSrc_Shock      = 2,
} LIS3_IntSrc_t;

typedef struct {
    LIS3_Res_t    resolution;
    LIS3_IntSrc_t int_src;
    uint8_t       scale_g;    // +/- 2, 4, 8 or 16G
    uint16_t      frequency;  // This will be truncated to the nearest available rate
    uint16_t      threshold;  // in mG
} LIS3_Config_t;

// All axes in mG
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} LIS3_Accel_t;

/*
 * PUBLIC FUNCTIONS
 */

bool LIS3_Init   (const LIS3_Config_t * cfg);
void LIS3_Deinit (void);
void LIS3_Read   (LIS3_Accel_t * acc);
bool LIS3_IsIntSet(void);

#endif // LIS3DH_H
