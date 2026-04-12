#include "app.h"

#include "tim.h"

#include "loop.h"
#include "encoder_driver.h"
#include "display.h"
#include "bmp280.h"

void app_init(void)
{
    //display_init();
    encoder_init();
    bmp280_init();
    loop_init();
}

void app_step(void)
{
    if (loop_take_step())
    {
        //display_step();
    }
}
    
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    loop_on_period_elapsed(htim);
}
