#include "ssd1306.h"
#include "ssd1306_tests.h"

void display_init(void)
{
    ssd1306_Init();
}

void display_step(void)
{
    ssd1306_TestCircle();

}
