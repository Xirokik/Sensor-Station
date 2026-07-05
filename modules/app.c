#include "app.h"

#include "tim.h"

#include "loop.h"
#include "encoder_driver.h"
#include "display.h"
#include "bmp280.h"
#include "console_app.h"
#include "hc_sr04.h"

void app_init(void)
{
    //display_init();
    encoder_init();
    bmp280_init();
    hc_sr04_init();
    console_app_init();
    loop_init();
}

void app_step(void)
{
    if (loop_take_step())
    {
        bmp280_update();
        (void)hc_sr04_get_dist();
        console_app_step();
        //display_step();
    }
}
    
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    loop_on_period_elapsed(htim);
}
