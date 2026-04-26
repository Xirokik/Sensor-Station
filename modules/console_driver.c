#include "console_driver.h"
// STD C
#include "stdint.h"
#include "stdbool.h"
// HAL
#include "usart.h"

#define CONSOLE_INTERFACE &huart2
#define RX_BUF_SIZE 128

static uint8_t rx_buff[RX_BUF_SIZE];
static volatile uint32_t head;
static volatile uint32_t tail;
static volatile bool tx_ready;
static volatile uint32_t lost_characters_cnt;

uint16_t console_driver_init(void)
{
	head = 0;
	tail = 0;
	tx_ready = true;
	// Start wainting for recive
	if(HAL_UART_Receive_IT(CONSOLE_INTERFACE, &rx_buff[head], 1) != HAL_OK) return CONSOLE_INIT_FAILED;
	return 0;
}

uint16_t console_driver_send(const void *tx_data, uint16_t len)
{
    if (tx_data == NULL || len == 0)
    {  
        return CONSOLE_BAD_ARG;
    }

    if (!tx_ready) return CONSOLE_TX_BUSY;

    tx_ready = false;
    if (HAL_UART_Transmit_DMA(CONSOLE_INTERFACE, (const uint8_t*) tx_data, len) != HAL_OK)
    	{
    		tx_ready = true;
    		return CONSOLE_TX_FAIL;
    	}

    return 0;
}

uint16_t console_driver_recv(uint8_t *rx_char)
{
	if (rx_char == NULL) return CONSOLE_BAD_ARG;
	if (tail == head) return CONSOLE_RX_BUF_EMPTY;

	*rx_char = rx_buff[tail];
	tail++;
	if(tail >= RX_BUF_SIZE)
	{
		tail = 0;
	}

	return 0;
}

uint32_t console_driver_lost_char(void)
{
	return lost_characters_cnt;
}

bool console_driver_is_tx_ready(void)
{
	return tx_ready;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == CONSOLE_INTERFACE)
    {
		uint32_t next_head = head + 1;

    	if(next_head >= RX_BUF_SIZE)
    	{
    		next_head = 0;
    	}

    	if(next_head == tail)
    	{
    		lost_characters_cnt++;
    	}
    	else
    	{
    		head = next_head;
    	}


    	HAL_UART_Receive_IT(CONSOLE_INTERFACE, &rx_buff[head], 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == CONSOLE_INTERFACE)
	{
		tx_ready = true;
	}
}
