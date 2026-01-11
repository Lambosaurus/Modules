#include "LIS3DH.h"

#include "GPIO.h"
#include "Core.h"

#ifdef LIS3_SPI
#include "SPI.h"
#else  // LIS3_I2C
#include "I2C.h"
#endif

/*
 * PRIVATE DEFINITIONS
 */

// I2C only
#ifdef LIS3_SPI

#define ADDR_WRITE      0x00
#define ADDR_READ       0x80
#define ADDR_BURST      0x40

#else // LIS3_I2C

// SA0 low (default address)
// If you strap SA0 high, change this to 0x19.
#define LIS3_ADDR       0x19 //0x18
#define ADDR_BURST      0x80

#endif

// Registers (LIS3DH datasheet)
#define REG_WHOAMI          0x0F
#define REG_TEMP_CFG        0x1F
#define REG_CTRL1           0x20
#define REG_CTRL2           0x21
#define REG_CTRL3           0x22
#define REG_CTRL4           0x23
#define REG_CTRL5           0x24
#define REG_CTRL6           0x25
#define REG_REFERENCE       0x26
#define REG_STATUS          0x27
#define REG_OUT_X_L         0x28
#define REG_OUT_X_H         0x29
#define REG_OUT_Y_L         0x2A
#define REG_OUT_Y_H         0x2B
#define REG_OUT_Z_L         0x2C
#define REG_OUT_Z_H         0x2D
#define REG_FIFO_CTRL       0x2E
#define REG_FIFO_SRC        0x2F

// Interrupt 1 (for wake / shock)
#define REG_INT1_CFG        0x30
#define REG_INT1_SRC        0x31
#define REG_INT1_THS        0x32
#define REG_INT1_DURATION   0x33

// Click / tap
#define REG_CLICK_CFG       0x38
#define REG_CLICK_SRC       0x39
#define REG_CLICK_THS       0x3A
#define REG_TIME_LIMIT      0x3B
#define REG_TIME_LATENCY    0x3C
#define REG_TIME_WINDOW     0x3D

#define WHOAMI_VALUE        0x33

/* CTRL_REG1 (20h) */
#define CR1_XEN             0x01
#define CR1_YEN             0x02
#define CR1_ZEN             0x04
#define CR1_LPEN            0x08  // low power enable
// ODR codes
#define CR1_ODR_POWERDOWN   0x00
#define CR1_ODR_1HZ         0x10
#define CR1_ODR_10HZ        0x20
#define CR1_ODR_25HZ        0x30
#define CR1_ODR_50HZ        0x40
#define CR1_ODR_100HZ       0x50
#define CR1_ODR_200HZ       0x60
#define CR1_ODR_400HZ       0x70
#define CR1_ODR_1600HZ      0x80 // 1.6 kHz

/* CTRL_REG2 (21h) - High-pass filter */
#define CR2_HPM_NORMAL_RESET    0x00
#define CR2_HPM_REF_SIG         0x40
#define CR2_HPCF_0              0x01
#define CR2_HPCF_1              0x02
#define CR2_HPCLICK             0x08 // HP filter for CLICK
#define CR2_HPIS1               0x04 // HP filter for INT1

/* CTRL_REG3 (22h) - Interrupts on INT1 */
#define CR3_I1_CLICK        0x80
#define CR3_I1_AOI1         0x40
#define CR3_I1_DRDY1        0x10
// (others not used)

/* CTRL_REG4 (23h) */
#define CR4_BDU             0x80
#define CR4_BLE             0x40
#define CR4_FS_2G           0x00
#define CR4_FS_4G           0x10
#define CR4_FS_8G           0x20
#define CR4_FS_16G          0x30
#define CR4_HR              0x08
// (self-test bits ignored)

/* CTRL_REG5 (24h) */
#define CR5_BOOT            0x80
#define CR5_FIFO_EN         0x40
#define CR5_LIR_INT1        0x08

/* CTRL_REG6 (25h) */
// We’ll route INT1 only; leave this at default 0

/* CLICK_CFG (38h) */
#define CLICK_XS            0x01
#define CLICK_XD            0x02
#define CLICK_YS            0x04
#define CLICK_YD            0x08
#define CLICK_ZS            0x10
#define CLICK_ZD            0x20

// The LIS2DT version had these commented out; keep the same style.
// (They don’t directly map 1:1 on LIS3DH but we keep the semantic idea.)
/*
#define WAKE_THS_TAP        0x80 // Enable single or double tap
#define WAKE_THS_SLEEP      0x40 // Enable sleep mode while waiting for threshold
#define WAKE_THS_MASK       0x3F // Threshold (in units of fs/64)

#define WAKE_DUR_FALL5      0x80
#define WAKE_DUR_MASK       0x60
#define WAKE_DIR_STATIONARY 0x10
#define WAKE_DUR_MASK       0x0F // Time to stay in wake mode (in 512 ODR)
*/

#define TAP_THS_MASK        0x7F  // CLICK_THS is 7 bits on LIS3DH

// For “shock” timing – keep same conceptual roles
#define TAP_DUR_SHOCK_MAX   0x03  // TIME_LIMIT (max shock duration in 1/ODR steps)
#define TAP_DUR_SHOCK_POS   0
#define TAP_DUR_QUIET_MAX   0x03  // TIME_LATENCY
#define TAP_DUR_QUIET_POS   0
#define TAP_DUR_LATENCY_MAX 0x0F  // TIME_WINDOW
#define TAP_DUR_LATENCY_POS 0

// Note the sign extension for the 16 bit number.
#define LIS3_ADC_TO_MG(b1, b2, fs)  (((int32_t)(int16_t)((b1) | ((b2) << 8)) * (fs)) >> 15)

#define PLACE_BITS(value, pos, max) ( ((value) & (max)) << (pos) )

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

#ifdef LIS3_SPI
static inline void LIS3_Select(void);
static inline void LIS3_Deselect(void);
#endif // LIS3_SPI

static void LIS3_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count);
static void LIS3_ReadRegs (uint8_t reg, uint8_t * data, uint8_t count);
static inline uint8_t LIS3_ReadReg (uint8_t reg);
static inline void   LIS3_WriteReg(uint8_t reg, uint8_t data);

static void LIS3_INT_IRQHandler(void);

static uint8_t LIS3_CR1_GetODR(uint16_t f);
static uint8_t LIS3_CR4_GetFS (uint8_t s);

/*
 * PRIVATE VARIABLES
 */

static struct {
    bool    int_set;
    bool    one_shot;
    uint8_t scale_g;
} gLis3;

/*
 * PUBLIC FUNCTIONS
 */

bool LIS3_Init(const LIS3_Config_t * cfg)
{
#ifdef LIS3_SPI
    GPIO_EnableOutput(LIS3_CS_PIN, GPIO_PIN_SET);
#endif

    gLis3.int_set  = false;
    gLis3.one_shot = false;

    GPIO_EnableInput(LIS3_INT_PIN, GPIO_Pull_Up);
    GPIO_OnChange(LIS3_INT_PIN, GPIO_IT_Falling, LIS3_INT_IRQHandler);

    bool success = (LIS3_ReadReg(REG_WHOAMI) == WHOAMI_VALUE);
    if (success)
    {
        // “Soft reset” / reboot memory
        LIS3_WriteReg(REG_CTRL5, CR5_BOOT);
        CORE_Delay(5);

        uint8_t cr1 = 0;
        uint8_t cr2 = 0;
        uint8_t cr3 = 0;
        uint8_t cr4 = 0;
        uint8_t cr5 = 0;
        uint8_t cr6 = 0;

        // We are in LP mode if the user has selected 12B.
        bool low_power = false;
        bool high_res  = false;

        switch (cfg->resolution)
        {
        case LIS3_Res_14B:
            // Use high-resolution mode on LIS3DH (12-bit, closest to your 14-bit intent)
            high_res = true;
            break;

        case LIS3_Res_12B:
            // Use low-power mode
            low_power = true;
            break;
        }

        gLis3.one_shot = (cfg->int_src == LIS3_IntSrc_None);
        gLis3.scale_g  = cfg->scale_g;

        // CTRL1: ODR + mode + axes
        cr1 |= LIS3_CR1_GetODR(cfg->frequency);
        cr1 |= CR1_XEN | CR1_YEN | CR1_ZEN;
        if (low_power) {
            cr1 |= CR1_LPEN;
        }

        // CTRL4: BDU + scale + high-resolution
        cr4 |= CR4_BDU;
        cr4 |= LIS3_CR4_GetFS(cfg->scale_g);
        if (high_res) {
            cr4 |= CR4_HR;
        }

        // Interrupt source selection
        if (cfg->int_src == LIS3_IntSrc_DataReady)
        {
            // INT1: DRDY1
            cr3 |= CR3_I1_DRDY1;

            // Keep HPF disabled for normal accel readout
            cr2 |= 0x00;
        }
        else if (cfg->int_src == LIS3_IntSrc_Shock)
        {
            // Use CLICK interrupt on INT1 (shock / tap style)
            cr3 |= CR3_I1_CLICK;

            // Enable HPF for CLICK + INT1
            cr2 |= CR2_HPM_NORMAL_RESET | CR2_HPCF_1 | CR2_HPCLICK | CR2_HPIS1;

            // Threshold is in units of FS/128 (datasheet); cfg->threshold is mG.
            // threshold_lsb ≈ threshold_mg / (FS_mg/128) = threshold_mg * 128 / (FS_g*1000)
            uint32_t full_scale_g = (uint32_t)cfg->scale_g;
            uint32_t thr_lsb = ((uint32_t)cfg->threshold * 128U) / (full_scale_g * 1000U);
            if (thr_lsb > TAP_THS_MASK) { thr_lsb = TAP_THS_MASK; }

            uint8_t click_ths = (uint8_t)thr_lsb;

            // Enable single-click on all axes (closest to LIS2 “shock” behaviour)
            uint8_t click_cfg = CLICK_XS | CLICK_YS | CLICK_ZS;

            // Shock duration, quiet time, window – mirror LIS2 idea with simple constants
            uint8_t time_limit   = PLACE_BITS(TAP_DUR_SHOCK_MAX,   TAP_DUR_SHOCK_POS,   TAP_DUR_SHOCK_MAX);
            uint8_t time_latency = PLACE_BITS(0,                   TAP_DUR_QUIET_POS,   TAP_DUR_QUIET_MAX);
            uint8_t time_window  = PLACE_BITS(0,                   TAP_DUR_LATENCY_POS, TAP_DUR_LATENCY_MAX);

            // Program click / tap block
            LIS3_WriteReg(REG_CLICK_THS,     click_ths);
            LIS3_WriteReg(REG_TIME_LIMIT,    time_limit);
            LIS3_WriteReg(REG_TIME_LATENCY,  time_latency);
            LIS3_WriteReg(REG_TIME_WINDOW,   time_window);
            LIS3_WriteReg(REG_CLICK_CFG,     click_cfg);
        }

        // For now we leave FIFO and latching off; CR5 used only for BOOT by us here.
        // If you were using wake / 6D features on LIS2DT, you’d map those into
        // INT1_CFG / ACT_THS / ACT_DUR here.

        LIS3_WriteReg(REG_CTRL1, cr1);
        LIS3_WriteReg(REG_CTRL2, cr2);
        LIS3_WriteReg(REG_CTRL3, cr3);
        LIS3_WriteReg(REG_CTRL4, cr4);
        LIS3_WriteReg(REG_CTRL5, cr5);
        LIS3_WriteReg(REG_CTRL6, cr6);

//        uint8_t cr[] = { cr1, cr2, cr3, cr4, cr5, cr6 };
//        LIS3_WriteRegs(REG_CTRL1, cr, sizeof(cr));

    }

    return success;
}

void LIS3_Deinit(void)
{
    GPIO_Deinit(LIS3_INT_PIN);
    GPIO_OnChange(LIS3_INT_PIN, GPIO_IT_Falling, LIS3_INT_IRQHandler);

    // Put device back into power-down
    uint8_t cr1 = CR1_ODR_POWERDOWN;
    LIS3_WriteReg(REG_CTRL1, cr1);
}

bool LIS3_IsIntSet(void)
{
    return gLis3.int_set;
}

void LIS3_Read(LIS3_Accel_t * acc)
{
    gLis3.int_set = false;

    uint8_t data[6];
    LIS3_ReadRegs(REG_OUT_X_L, data, sizeof(data));

    int32_t full_scale = (int32_t)gLis3.scale_g * 1000;

    acc->x = (int16_t)LIS3_ADC_TO_MG(data[0], data[1], full_scale);
    acc->y = (int16_t)LIS3_ADC_TO_MG(data[2], data[3], full_scale);
    acc->z = (int16_t)LIS3_ADC_TO_MG(data[4], data[5], full_scale);
}

/*
 * PRIVATE FUNCTIONS
 */

static uint8_t LIS3_CR1_GetODR(uint16_t f)
{
    if      (f == 0)       { return CR1_ODR_POWERDOWN; }
    else if (f < 10)       { return CR1_ODR_1HZ;       }
    else if (f < 25)       { return CR1_ODR_10HZ;      }
    else if (f < 50)       { return CR1_ODR_25HZ;      }
    else if (f < 100)      { return CR1_ODR_50HZ;      }
    else if (f < 200)      { return CR1_ODR_100HZ;     }
    else if (f < 400)      { return CR1_ODR_200HZ;     }
    else if (f < 800)      { return CR1_ODR_400HZ;     }
    else                   { return CR1_ODR_1600HZ;    }
}

static uint8_t LIS3_CR4_GetFS(uint8_t s)
{
    if      (s < 4)        { return CR4_FS_2G;  }
    else if (s < 8)        { return CR4_FS_4G;  }
    else if (s < 16)       { return CR4_FS_8G;  }
    else                   { return CR4_FS_16G; }
}

static inline uint8_t LIS3_ReadReg(uint8_t reg)
{
    uint8_t v;
    LIS3_ReadRegs(reg, &v, 1);
    return v;
}

static inline void LIS3_WriteReg(uint8_t reg, uint8_t data)
{
    LIS3_WriteRegs(reg, &data, 1);
}

#ifdef LIS3_SPI

static void LIS3_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count)
{
    uint8_t header = reg | ADDR_WRITE | ((count > 1) ? ADDR_BURST : 0x00);
    LIS3_Select();
    SPI_Write(LIS3_SPI, &header, 1);
    SPI_Write(LIS3_SPI, data, count);
    LIS3_Deselect();
}

static void LIS3_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
    uint8_t header = reg | ADDR_READ | ((count > 1) ? ADDR_BURST : 0x00);
    LIS3_Select();
    SPI_Write(LIS3_SPI, &header, 1);
    SPI_Read(LIS3_SPI, data, count);
    LIS3_Deselect();
}

static inline void LIS3_Select(void)
{
    GPIO_Reset(LIS3_CS_PIN);
}

static inline void LIS3_Deselect(void)
{
    GPIO_Set(LIS3_CS_PIN);
}

#else // LIS3_I2C

static void LIS3_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count)
{
    // Ignore the error

    uint8_t tx[count + 1];
    tx[0] = reg | ((count > 1) ? ADDR_BURST : 0x00);
    memcpy(tx+1, data, count);
    I2C_Write(LIS3_I2C, LIS3_ADDR, tx, count + 1);
}

static void LIS3_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
    uint8_t tx = reg | ((count > 1) ? ADDR_BURST : 0x00);
    if (!I2C_Transfer(LIS3_I2C, LIS3_ADDR, &tx, 1, data, count))
    {
        // If the I2C transfer failed - then zero everything out to at least make behavior well defined.
        bzero(data, count);
    }
}

#endif

/*
 * INTERRUPT ROUTINES
 */

static void LIS3_INT_IRQHandler(void)
{
    gLis3.int_set = true;
}
