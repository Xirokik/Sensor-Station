#include "loop.h"

#include <stdint.h>
#include <stdbool.h>

#include "tim.h"

#define LOOP_TIM &htim6

static volatile uint32_t all_steps;
static uint32_t missed_steps;
static uint32_t cur_step;

uint16_t loop_init(void)
{
	if (HAL_TIM_Base_Start_IT(LOOP_TIM) != HAL_OK) return LOOP_FLT_INIT;
	return 0;
}

bool loop_take_step(void)
{
	uint32_t steps_snapshot = all_steps;
	uint32_t diff = steps_snapshot - cur_step;
	if (diff == 0)
	{
		return false;
	}

	if (diff > 1)
	{
		missed_steps += diff - 1;
	}

	cur_step = steps_snapshot;
	return true;
}

uint32_t loop_get_all_steps(void)
{
	return all_steps;
}

uint32_t loop_get_missed_steps(void)
{
	return missed_steps;
}

void loop_on_period_elapsed(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == (LOOP_TIM)->Instance)
    {
        all_steps++;
    }
}
