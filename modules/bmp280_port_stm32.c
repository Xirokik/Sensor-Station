#include "bmp280_port.h"

#include "spi.h"
#include "gpio.h"
#include "stm32l4xx_hal_gpio.h"

#define BMP_SPI hspi3
#define BMP_NCS_PORT BMP_NCS_GPIO_Port
#define BMP_NCS_PIN BMP_NCS_Pin

#define BMP280_SPI_TIMEOUT_MS 5U

uint32_t bmp280_port_write(uint8_t reg, const uint8_t *data, uint16_t len)
{
    uint8_t tx_buff[len + 1];
    tx_buff[0] = reg & 0x7F;
    for (uint32_t i = 0; i < len; i++)
    {
        tx_buff[i + 1] = data[i];
    }

    HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_RESET);
    if (HAL_SPI_Transmit(&BMP_SPI, tx_buff, len + 1, BMP280_SPI_TIMEOUT_MS) != HAL_OK)
    {
        HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);
        return 1;
    }

    HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);
    return 0;
}

uint32_t bmp280_port_read(uint8_t reg, uint8_t *data, uint16_t len)
{
    uint8_t tx_buf[1 + len];
    uint8_t rx_buf[1 + len];

    tx_buf[0] = reg | 0x80;

    for (uint16_t i = 1; i < (len + 1); i++)
    {
        tx_buf[i] = 0xFF;
    }

    HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_RESET);

    if (HAL_SPI_TransmitReceive(&BMP_SPI, tx_buf, rx_buf, len + 1, BMP280_SPI_TIMEOUT_MS) != HAL_OK)
    {
        HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);
        return 1;
    }

    HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);

    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = rx_buf[i + 1];
    }

    return 0;
}