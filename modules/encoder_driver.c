#include "encoder_driver.h"

#include "stdint.h"
#include "stdbool.h"
#include "tim.h"
#include "gpio.h"

#define ENCODER_DETENTS 20
#define ENCODER_EDGES_PER_STEP 4
#define ENCODER_COUNTER_PERIOD (ENCODER_DETENTS * ENCODER_EDGES_PER_STEP - 1)

#define ENCODER_TIM &htim4
#define ENCODER_CHANNELS (TIM_CHANNEL_1 | TIM_CHANNEL_2)
#define ENCODER_SHIFT 2
#define ENCODER_BTN_PORT ENC_BTN_GPIO_Port
#define ENCODER_BTN_PIN ENC_BTN_Pin

uint16_t encoder_init(void)
{
	__HAL_TIM_SET_AUTORELOAD(ENCODER_TIM, ENCODER_COUNTER_PERIOD);
	if(HAL_TIM_Encoder_Start(ENCODER_TIM, ENCODER_CHANNELS) != HAL_OK) return ENCODER_FLT_INIT;
	return 0;
}

uint32_t encoder_get_pos(void)
{
	uint32_t enc_raw =__HAL_TIM_GET_COUNTER(ENCODER_TIM);
	uint32_t enc = enc_raw >> ENCODER_SHIFT;
	return enc;
}

bool is_encoder_button_pressed(void)
{
	return HAL_GPIO_ReadPin(ENCODER_BTN_PORT, ENCODER_BTN_PIN) == GPIO_PIN_RESET;
}