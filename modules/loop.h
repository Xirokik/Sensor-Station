#ifndef LOOP_H_
#define LOOP_H_

#include <stdint.h>
#include <stdbool.h>

#include "tim.h"

#define LOOP_FLT_INIT 0x120

uint16_t loop_init(void);

bool loop_take_step(void);

uint32_t loop_get_all_steps(void);

uint32_t loop_get_missed_steps(void);

void loop_on_period_elapsed(TIM_HandleTypeDef *htim);

#endif /* LOOP_H_ */
