#include "hc_sr04_driver.h"

#include "stdint.h"
#include "tim.h"

#define HC_SR04_MAX_DIST        200.0f
#define HC_SR04_MIN_DIST        2.0f
#define HC_SR04_INVALID_DIST   -1.0f

volatile static uint32_t val_ch1;
volatile static uint32_t val_ch2;
volatile static uint32_t channel_diff;

void hc_sr04_driver_init(void)
{
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
}

float hc_sr04_driver_get_dist(void)
{
    float distance_cm = (float)(channel_diff) / 58.0f;
    if (distance_cm > HC_SR04_MAX_DIST || distance_cm < HC_SR04_MIN_DIST)
    {
        distance_cm = HC_SR04_INVALID_DIST;
    }
    
    return distance_cm;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == htim2.Instance)
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            val_ch1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        }
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
        {
            val_ch2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
            channel_diff = val_ch2 - val_ch1;
        }
    }
}
