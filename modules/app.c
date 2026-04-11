#include "app.h"

#include <stdint.h>

#include "loop.h"
#include "encoder_driver.h"

void app_init(void)
{
    encoder_init();
    loop_init();
}

void app_step(void)
{
    if (is_step_ready())
    {
        step_done();
    }
    
}