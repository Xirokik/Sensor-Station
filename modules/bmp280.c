#include "bmp280.h"

#include <stdint.h>

#include "bmp280_port.h"
#include "stm32l4xx_hal.h"

#define BMP280_REG_ID 0xD0
#define BMP280_REG_RESET 0xE0
#define BMP280_REG_STATUS 0xF3
#define BMP280_REG_CALIB00 0x88
#define BMP280_REG_CONFIG 0xF5
#define BMP280_REG_CTRL_MEAS 0xF4
#define BMP280_REG_PRESS_MSB 0xF7

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
  
} bmp280_t;

bmp280_t bmp280;



uint32_t bmp280_init(void)
{
    uint8_t ret_val;
    bmp280_port_read(BMP280_REG_ID, &ret_val, 1);
    if (ret_val != 0x58)
    {
        return 1;
    }
    uint8_t reset_val = 0xB6;
    bmp280_port_write(BMP280_REG_RESET, &reset_val, 1);

    uint8_t status = 0;
    do
    {
        if (bmp280_port_read(BMP280_REG_STATUS, &status, 1) != 0)
        {
            return 2;
        }
    }
    while (status & 0x01);

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
    uint8_t config = (0 << 5) | (2 << 2) | (0 << 0);

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
    uint8_t ctrl_meas = (1 << 5) | (3 << 2) | (1 << 0);

    if (bmp280_port_write(BMP280_REG_CTRL_MEAS, &ctrl_meas, 1) != 0)
    {
        return 1;
    }

    //HAL_Delay(100);

    do
    {
        bmp280_port_read(BMP280_REG_STATUS, &status, 1);
    } while (status & 0x08);
    uint8_t raw[6];

    bmp280_port_read(BMP280_REG_PRESS_MSB, raw, 6);

    int32_t adc_P = (int32_t)(((uint32_t)raw[0] << 12) |
                              ((uint32_t)raw[1] << 4)  |
                              ((uint32_t)raw[2] >> 4));

    int32_t adc_T = (int32_t)(((uint32_t)raw[3] << 12) |
                              ((uint32_t)raw[4] << 4)  |
                              ((uint32_t)raw[5] >> 4));

    return 0;
}