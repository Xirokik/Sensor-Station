#include "bmp280_port.h"

#include "spi.h"
#include "gpio.h"
#include "stm32l4xx_hal_gpio.h"

#define BMP_SPI hspi3
#define BMP_NCS_PORT BMP_NCS_GPIO_Port
#define BMP_NCS_PIN BMP_NCS_Pin

#define BMP280_SPI_TIMEOUT_MS 5U
#define BMP280_PORT_BUFFER_SIZE 32U

static volatile bool bmp280_port_transfer_ok = false;
static uint8_t bmp280_port_tx_buf[BMP280_PORT_BUFFER_SIZE + 1U];
static uint8_t bmp280_port_rx_buf[BMP280_PORT_BUFFER_SIZE + 1U];
static uint8_t *bmp280_port_read_dst;
static uint16_t bmp280_port_read_len;

static bool bmp280_port_can_start_transfer(uint16_t len, const uint8_t *data);
static void bmp280_port_finish_error(void);

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
        bmp280_port_transfer_ok = false;
        HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);
        return 1;
    }

    HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);
    bmp280_port_transfer_ok = true;
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
        bmp280_port_transfer_ok = false;
        HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);
        return 1;
    }

    HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);
    bmp280_port_transfer_ok = true;
    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = rx_buf[i + 1];
    }

    return 0;
}

uint32_t bmp280_port_write_it(uint8_t reg, const uint8_t *data, uint16_t len)
{
    if (!bmp280_port_can_start_transfer(len, data))
    {
        return 1;
    }

    bmp280_port_tx_buf[0] = reg & 0x7FU;
    for (uint16_t i = 0; i < len; i++)
    {
        bmp280_port_tx_buf[i + 1U] = data[i];
    }

    bmp280_port_read_dst = NULL;
    bmp280_port_read_len = 0U;

    HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_RESET);
    if (HAL_SPI_Transmit_IT(&BMP_SPI, bmp280_port_tx_buf, len + 1U) != HAL_OK)
    {
        bmp280_port_finish_error();
        return 1;
    }

    return 0;
}

uint32_t bmp280_port_read_it(uint8_t reg, uint8_t *data, uint16_t len)
{
    if (!bmp280_port_can_start_transfer(len, data))
    {
        return 1;
    }

    bmp280_port_tx_buf[0] = reg | 0x80U;
    bmp280_port_rx_buf[0] = 0U;
    for (uint16_t i = 0; i < len; i++)
    {
        bmp280_port_tx_buf[i + 1U] = 0xFFU;
        bmp280_port_rx_buf[i + 1U] = 0U;
    }

    bmp280_port_read_dst = data;
    bmp280_port_read_len = len;

    HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_RESET);
    if (HAL_SPI_TransmitReceive_IT(&BMP_SPI, bmp280_port_tx_buf, bmp280_port_rx_buf, len + 1U) != HAL_OK)
    {
        bmp280_port_finish_error();
        return 1;
    }

    return 0;
}

bool bmp280_port_is_ready(void)
{
    return HAL_SPI_GetState(&BMP_SPI) == HAL_SPI_STATE_READY;
}

bool bmp280_port_last_transfer_ok(void)
{
    return bmp280_port_transfer_ok;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == BMP_SPI.Instance)
    {
        HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);
        bmp280_port_transfer_ok = true;
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == BMP_SPI.Instance)
    {
        HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);
        if (bmp280_port_read_dst != NULL)
        {
            for (uint16_t i = 0; i < bmp280_port_read_len; i++)
            {
                bmp280_port_read_dst[i] = bmp280_port_rx_buf[i + 1U];
            }
        }

        bmp280_port_transfer_ok = true;
        bmp280_port_read_dst = NULL;
        bmp280_port_read_len = 0U;
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == BMP_SPI.Instance)
    {
        bmp280_port_finish_error();
    }
}

static bool bmp280_port_can_start_transfer(uint16_t len, const uint8_t *data)
{
    if ((data == NULL) || (len == 0U) || (len > BMP280_PORT_BUFFER_SIZE))
    {
        return false;
    }

    if (!bmp280_port_is_ready())
    {
        return false;
    }

    return true;
}

static void bmp280_port_finish_error(void)
{
    HAL_GPIO_WritePin(BMP_NCS_PORT, BMP_NCS_PIN, GPIO_PIN_SET);
    bmp280_port_transfer_ok = false;
    bmp280_port_read_dst = NULL;
    bmp280_port_read_len = 0U;
}
