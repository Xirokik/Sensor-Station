#ifndef BMP280_REGS_H_
#define BMP280_REGS_H_

/* BMP280 register map */
#define BMP280_REG_CALIB00             0x88
#define BMP280_REG_CHIP_ID             0xD0
#define BMP280_REG_RESET               0xE0
#define BMP280_REG_STATUS              0xF3
#define BMP280_REG_CTRL_MEAS           0xF4
#define BMP280_REG_CONFIG              0xF5
#define BMP280_REG_PRESS_MSB           0xF7
#define BMP280_REG_PRESS_LSB           0xF8
#define BMP280_REG_PRESS_XLSB          0xF9
#define BMP280_REG_TEMP_MSB            0xFA
#define BMP280_REG_TEMP_LSB            0xFB
#define BMP280_REG_TEMP_XLSB           0xFC

/* Fixed values */
#define BMP280_CHIP_ID                 0x58
#define BMP280_RESET_VALUE             0xB6

/* STATUS register bits */
#define BMP280_STATUS_IM_UPDATE_Pos    0U
#define BMP280_STATUS_IM_UPDATE_Msk    (1U << BMP280_STATUS_IM_UPDATE_Pos)
#define BMP280_STATUS_MEASURING_Pos    3U
#define BMP280_STATUS_MEASURING_Msk    (1U << BMP280_STATUS_MEASURING_Pos)

/* CTRL_MEAS register fields */
#define BMP280_CTRL_MEAS_MODE_Pos      0U
#define BMP280_CTRL_MEAS_MODE_Msk      (0x03U << BMP280_CTRL_MEAS_MODE_Pos)
#define BMP280_CTRL_MEAS_OSRS_P_Pos    2U
#define BMP280_CTRL_MEAS_OSRS_P_Msk    (0x07U << BMP280_CTRL_MEAS_OSRS_P_Pos)
#define BMP280_CTRL_MEAS_OSRS_T_Pos    5U
#define BMP280_CTRL_MEAS_OSRS_T_Msk    (0x07U << BMP280_CTRL_MEAS_OSRS_T_Pos)

#define BMP280_MODE_SLEEP              (0x00U << BMP280_CTRL_MEAS_MODE_Pos)
#define BMP280_MODE_FORCED             (0x01U << BMP280_CTRL_MEAS_MODE_Pos)
#define BMP280_MODE_NORMAL             (0x03U << BMP280_CTRL_MEAS_MODE_Pos)

#define BMP280_OSRS_SKIPPED            0x00U
#define BMP280_OSRS_X1                 0x01U
#define BMP280_OSRS_X2                 0x02U
#define BMP280_OSRS_X4                 0x03U
#define BMP280_OSRS_X8                 0x04U
#define BMP280_OSRS_X16                0x05U

#define BMP280_CTRL_MEAS_OSRS_T(value) ((uint8_t)((value) << BMP280_CTRL_MEAS_OSRS_T_Pos))
#define BMP280_CTRL_MEAS_OSRS_P(value) ((uint8_t)((value) << BMP280_CTRL_MEAS_OSRS_P_Pos))

/* CONFIG register fields */
#define BMP280_CONFIG_SPI3W_EN_Pos     0U
#define BMP280_CONFIG_SPI3W_EN_Msk     (0x01U << BMP280_CONFIG_SPI3W_EN_Pos)
#define BMP280_CONFIG_FILTER_Pos       2U
#define BMP280_CONFIG_FILTER_Msk       (0x07U << BMP280_CONFIG_FILTER_Pos)
#define BMP280_CONFIG_T_SB_Pos         5U
#define BMP280_CONFIG_T_SB_Msk         (0x07U << BMP280_CONFIG_T_SB_Pos)

#define BMP280_FILTER_OFF              0x00U
#define BMP280_FILTER_X2               0x01U
#define BMP280_FILTER_X4               0x02U
#define BMP280_FILTER_X8               0x03U
#define BMP280_FILTER_X16              0x04U

#define BMP280_T_SB_0_5_MS             0x00U
#define BMP280_T_SB_62_5_MS            0x01U
#define BMP280_T_SB_125_MS             0x02U
#define BMP280_T_SB_250_MS             0x03U
#define BMP280_T_SB_500_MS             0x04U
#define BMP280_T_SB_1000_MS            0x05U
#define BMP280_T_SB_2000_MS            0x06U
#define BMP280_T_SB_4000_MS            0x07U

#define BMP280_CONFIG_FILTER(value)    ((uint8_t)((value) << BMP280_CONFIG_FILTER_Pos))
#define BMP280_CONFIG_T_SB(value)      ((uint8_t)((value) << BMP280_CONFIG_T_SB_Pos))

#endif /* BMP280_REGS_H_ */
