#include "app.h"

#include "loop.h"
#include "encoder_driver.h"
#include "tim.h"

void app_init(void)
{
    encoder_init();
    loop_init();
}

void app_step(void)
{
    if (loop_take_step())
    {
    }
}
    
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    loop_on_period_elapsed(htim);
}
