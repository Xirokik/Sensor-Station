#include "loop.h"

#include <stdint.h>
#include <stdbool.h>

#include "tim.h"

#define LOOP_TIM &htim6

static volatile bool step_ready = false;
static volatile uint32_t all_steps;
static uint32_t missed_steps;
static uint32_t cur_step;

uint16_t loop_init(void)
{
	if (HAL_TIM_Base_Start_IT(LOOP_TIM) != HAL_OK) return LOOP_FLT_INIT;
	return 0;
}

bool is_step_ready(void)
{
	uint32_t diff = all_steps - cur_step;
	if (diff > 1)
	{
		missed_steps += diff - 1;
	}

	cur_step = all_steps;
	return step_ready;
}

void step_done(void)
{
	step_ready = false;
}

uint32_t loop_get_all_steps(void)
{
	return all_steps;
}

uint32_t loop_get_missed_steps(void)
{
	return missed_steps;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == (LOOP_TIM)->Instance)
    {
        step_ready = true;
        all_steps++;
    }
}
