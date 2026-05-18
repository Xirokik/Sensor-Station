#include "bmp280.h"

#include <stdint.h>
#include <stdbool.h>

#include "bmp280_regs.h"
#include "bmp280_port.h"

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
    uint8_t raw_data[6];
    BMP280_S32_t t_fine;
    BMP280_S32_t temp_raw;
    BMP280_U32_t pres_Pa;
    float temp_C;
    float pres_hPa;
    bool init_ok;
} bmp280_t;

static bmp280_t bmp280;

static int32_t bmp280_calc_temp(BMP280_S32_t adc_T);
static uint32_t bmp280_calc_press(BMP280_S32_t adc_P);

void bmp280_init(void)
{
    bmp280.init_ok = false;
    uint8_t ret_val = 0;
    if (bmp280_port_read(BMP280_REG_CHIP_ID, &ret_val, 1) != 0)
    {
        return;
    }
    else if (ret_val != BMP280_CHIP_ID)
    {
        return;
    }
    uint8_t reset_val = BMP280_RESET_VALUE;
    if (bmp280_port_write(BMP280_REG_RESET, &reset_val, 1) != 0)
    {
        return;
    }
    uint32_t bmp280_timeout = 100000;
    uint8_t status = 0;
    do
    {
        if (bmp280_port_read(BMP280_REG_STATUS, &status, 1) != 0)
        {
            return;
        }

        if (--bmp280_timeout == 0)
        {
            return;
        }

    } while ((status & BMP280_STATUS_IM_UPDATE_Msk) != 0);

    uint8_t calib[24];
    if (bmp280_port_read(BMP280_REG_CALIB00, calib, 24) != 0)
    {
        return;
    }

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

    uint8_t config = BMP280_CONFIG_T_SB(BMP280_T_SB_0_5_MS) |
                     BMP280_CONFIG_FILTER(BMP280_FILTER_X4);

    if (bmp280_port_write(BMP280_REG_CONFIG, &config, 1) != 0)
    {
        return;
    }
    bmp280.mode = BMP280_CTRL_MEAS_OSRS_T(BMP280_OSRS_X1) |
                  BMP280_CTRL_MEAS_OSRS_P(BMP280_OSRS_X4) | BMP280_MODE_NORMAL;

    if (bmp280_port_write(BMP280_REG_CTRL_MEAS, &bmp280.mode, 1) != 0)
    {
        return;
    }

    if (bmp280_port_read_it(BMP280_REG_PRESS_MSB, bmp280.raw_data, 6) != 0)
    {
        return;
    }
    
    bmp280.init_ok = true;
    return;
}

void bmp280_update(void)
{
    if (!bmp280.init_ok || !bmp280_port_is_ready())
    {
        return;
    }

    if (!bmp280_port_last_transfer_ok())
    {
        (void)bmp280_port_read_it(BMP280_REG_PRESS_MSB, bmp280.raw_data, 6);
        return;
    }

    int32_t adc_P = (int32_t)(((uint32_t)bmp280.raw_data[0] << 12) |
                              ((uint32_t)bmp280.raw_data[1] << 4) |
                              ((uint32_t)bmp280.raw_data[2] >> 4));

    int32_t adc_T = (int32_t)(((uint32_t)bmp280.raw_data[3] << 12) |
                              ((uint32_t)bmp280.raw_data[4] << 4) |
                              ((uint32_t)bmp280.raw_data[5] >> 4));

    bmp280.temp_raw = bmp280_calc_temp(adc_T);
    bmp280.pres_Pa = bmp280_calc_press(adc_P);
    bmp280.temp_C = bmp280.temp_raw / 100.0f;
    bmp280.pres_hPa = bmp280.pres_Pa / 100.0f;

    if (bmp280_port_read_it(BMP280_REG_PRESS_MSB, bmp280.raw_data, 6) != 0)
    {
        return;
    }
}

bool bmp280_is_init(void)
{
    return bmp280.init_ok;
}

float bmp280_get_temp_C(void)
{
    return bmp280.temp_C;
}

uint32_t bmp280_get_pres_Pa(void)
{
    return bmp280.pres_Pa;
}

float bmp280_get_pres_hPa(void)
{
    return bmp280.pres_hPa;
}

// Compensation formulas from Bosch BMP280 datasheet.
static int32_t bmp280_calc_temp(BMP280_S32_t adc_T)
{
    BMP280_S32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((BMP280_S32_t)bmp280.calib.dig_T1 << 1))) * ((BMP280_S32_t)bmp280.calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((BMP280_S32_t)bmp280.calib.dig_T1)) * ((adc_T >> 4) - ((BMP280_S32_t)bmp280.calib.dig_T1))) >> 12) * ((BMP280_S32_t)bmp280.calib.dig_T3)) >> 14;
    bmp280.t_fine = var1 + var2;
    T = (bmp280.t_fine * 5 + 128) >> 8;
    return T;
}

static uint32_t bmp280_calc_press(BMP280_S32_t adc_P)
{
    BMP280_S32_t var1, var2;
    BMP280_U32_t p;
    var1 = (((BMP280_S32_t)bmp280.t_fine) >> 1) - (BMP280_S32_t)64000;
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
