#include "bmp280.h"

#include <stdint.h>

#include "bmp280_regs.h"
#include "bmp280_port.h"
#include "stm32l4xx_hal.h"

#define BMP280_S32_t int32_t
#define BMP280_U32_t uint32_t

typedef struct
{
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
} bmp280_calib_data_t;

typedef struct
{
    uint8_t mode;
    bmp280_calib_data_t calib;
    uint8_t bmp_status;
    BMP280_S32_t t_fine;
    BMP280_S32_t temp_raw;
    BMP280_U32_t pres_raw;
    float temp;
    float pres;
} bmp280_t;

bmp280_t bmp280;

static int32_t bmp280_calc_temp(uint32_t *t_fine, BMP280_S32_t adc_T);
static uint32_t bmp280_calc_press(uint32_t t_fine, BMP280_S32_t adc_P);

uint32_t bmp280_init(void)
{
    uint8_t ret_val;
    bmp280_port_read(BMP280_REG_CHIP_ID, &ret_val, 1);
    if (ret_val != BMP280_CHIP_ID)
    {
        return 1;
    }
    uint8_t reset_val = BMP280_RESET_VALUE;
    bmp280_port_write(BMP280_REG_RESET, &reset_val, 1);

    uint8_t status = 0;
    do
    {
        if (bmp280_port_read(BMP280_REG_STATUS, &status, 1) != 0)
        {
            return 2;
        }
    } while (status & BMP280_STATUS_IM_UPDATE_Msk);

    uint8_t calib[24];
    bmp280_port_read(BMP280_REG_CALIB00, calib, 24);

    bmp280.calib.dig_T1 = (uint16_t)(calib[1] << 8 | calib[0]);
    bmp280.calib.dig_T2 = (int16_t)(calib[3] << 8 | calib[2]);
    bmp280.calib.dig_T3 = (int16_t)(calib[5] << 8 | calib[4]);
    bmp280.calib.dig_P1 = (uint16_t)(calib[7] << 8 | calib[6]);
    bmp280.calib.dig_P2 = (int16_t)(calib[9] << 8 | calib[8]);
    bmp280.calib.dig_P3 = (int16_t)(calib[11] << 8 | calib[10]);
    bmp280.calib.dig_P4 = (int16_t)(calib[13] << 8 | calib[12]);
    bmp280.calib.dig_P5 = (int16_t)(calib[15] << 8 | calib[14]);
    bmp280.calib.dig_P6 = (int16_t)(calib[17] << 8 | calib[16]);
    bmp280.calib.dig_P7 = (int16_t)(calib[19] << 8 | calib[18]);
    bmp280.calib.dig_P8 = (int16_t)(calib[21] << 8 | calib[20]);
    bmp280.calib.dig_P9 = (int16_t)(calib[23] << 8 | calib[22]);
    /*
     * CONFIG register:
     * t_sb     = 0b000 -> 0.5 ms standby
     * filter   = 0b010 -> IIR filter x4
     * spi3w_en = 0     -> 4-wire SPI
     */
    uint8_t config = BMP280_CONFIG_T_SB(BMP280_T_SB_0_5_MS) |
                     BMP280_CONFIG_FILTER(BMP280_FILTER_X4);

    if (bmp280_port_write(BMP280_REG_CONFIG, &config, 1) != 0)
    {
        return 1;
    }

    /*
     * CTRL_MEAS register:
     * osrs_t = 001 -> temperature oversampling x1
     * osrs_p = 011 -> pressure oversampling x4
     * mode   = 01  -> forced mode
     */
    uint8_t ctrl_meas = BMP280_CTRL_MEAS_OSRS_T(BMP280_OSRS_X1) |
                        BMP280_CTRL_MEAS_OSRS_P(BMP280_OSRS_X4) |
                        BMP280_MODE_FORCED;

    if (bmp280_port_write(BMP280_REG_CTRL_MEAS, &ctrl_meas, 1) != 0)
    {
        return 1;
    }

    HAL_Delay(100);

    do
    {
        bmp280_port_read(BMP280_REG_STATUS, &status, 1);
    } while (status & BMP280_STATUS_MEASURING_Msk);
    uint8_t raw[6];
    bmp280_port_read(BMP280_REG_PRESS_MSB, raw, 6);

    int32_t adc_P = (int32_t)(((uint32_t)raw[0] << 12) |
                              ((uint32_t)raw[1] << 4) |
                              ((uint32_t)raw[2] >> 4));

    int32_t adc_T = (int32_t)(((uint32_t)raw[3] << 12) |
                              ((uint32_t)raw[4] << 4) |
                              ((uint32_t)raw[5] >> 4));
    uint32_t t_fine = 0;
    bmp280.temp_raw = bmp280_calc_temp(&bmp280.t_fine, adc_T);
    bmp280.pres_raw = bmp280_calc_press(t_fine, adc_P);
    bmp280.temp = bmp280.temp_raw / 100.0f;
    bmp280.pres = bmp280.pres_raw / 100.0f;
    return 0;
}

static int32_t bmp280_calc_temp(uint32_t *t_fine, BMP280_S32_t adc_T)
{
    BMP280_S32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((BMP280_S32_t)bmp280.calib.dig_T1 << 1))) * ((BMP280_S32_t)bmp280.calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((BMP280_S32_t)bmp280.calib.dig_T1)) * ((adc_T >> 4) - ((BMP280_S32_t)bmp280.calib.dig_T1))) >> 12) * ((BMP280_S32_t)bmp280.calib.dig_T3)) >> 14;
    *t_fine = var1 + var2;
    T = (*t_fine * 5 + 128) >> 8;
    return T;
}

static uint32_t bmp280_calc_press(uint32_t t_fine, BMP280_S32_t adc_P)
{
    BMP280_S32_t var1, var2;
    BMP280_U32_t p;
    var1 = (((BMP280_S32_t)t_fine) >> 1) - (BMP280_S32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((BMP280_S32_t)bmp280.calib.dig_P6);
    var2 = var2 + ((var1 * ((BMP280_S32_t)bmp280.calib.dig_P5)) << 1);
    var2 = (var2 >> 2) + (((BMP280_S32_t)bmp280.calib.dig_P4) << 16);
    var1 = (((bmp280.calib.dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((BMP280_S32_t)bmp280.calib.dig_P2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((BMP280_S32_t)bmp280.calib.dig_P1)) >> 15);
    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = (((BMP280_U32_t)(((BMP280_S32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;
    if (p < 0x80000000)
    {
        p = (p << 1) / ((BMP280_U32_t)var1);
    }
    else
    {
        p = (p / (BMP280_U32_t)var1) * 2;
    }
    var1 = (((BMP280_S32_t)bmp280.calib.dig_P9) * ((BMP280_S32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((BMP280_S32_t)(p >> 2)) * ((BMP280_S32_t)bmp280.calib.dig_P8)) >> 13;
    p = (BMP280_U32_t)((BMP280_S32_t)p + ((var1 + var2 + bmp280.calib.dig_P7) >> 4));
    return p;
}
