
#include "console_app.h"

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "stddef.h"
#include "stdbool.h"

#include "console_driver.h"
#include "bmp280.h"

// Reset
#define CONSOLE_RESET       	"\033[0m"

// Text Color
#define CONSOLE_BLACK       	"\033[30m"
#define CONSOLE_RED         	"\033[31m"
#define CONSOLE_GREEN       	"\033[32m"
#define CONSOLE_YELLOW      	"\033[33m"
#define CONSOLE_BLUE        	"\033[34m"
#define CONSOLE_MAGENTA     	"\033[35m"
#define CONSOLE_CYAN        	"\033[36m"
#define CONSOLE_WHITE       	"\033[37m"

// Background Color
#define CONSOLE_BG_BLACK    	"\033[40m"
#define CONSOLE_BG_RED      	"\033[41m"
#define CONSOLE_BG_GREEN    	"\033[42m"
#define CONSOLE_BG_YELLOW   	"\033[43m"
#define CONSOLE_BG_BLUE     	"\033[44m"
#define CONSOLE_BG_MAGENTA  	"\033[45m"
#define CONSOLE_BG_CYAN     	"\033[46m"
#define CONSOLE_BG_WHITE    	"\033[47m"

// Text Style
#define CONSOLE_BOLD        	"\033[1m"
#define CONSOLE_UNDERLINE   	"\033[4m"
#define CONSOLE_REVERSE     	"\033[7m"

#define CONSOLE_CLEAR_SCREEN 	"\033[2J"
#define CONSOLE_CURSOR_HOME   	"\033[H"

#define BUFFOR_SIZE 1024

#define CONSOLE_REDRAW_PERIOD_STEPS 30U

typedef enum
{
	CONSOLE_MENU = 0,
	CONSOLE_SCREEN1 = 1,
	CONSOLE_ERROR = 2 // TBD
} console_screen_t;

static console_screen_t console_screen = CONSOLE_MENU;

static char buf[BUFFOR_SIZE];
static char *p_buf = buf;
static bool redraw = false;
static bool init_ok;
static uint32_t redraw_step_cnt;

static void screen_handle(uint8_t cmd);
static void screen_draw(void);

static inline size_t console_buf_left(void)
{
    if (p_buf < buf || p_buf >= buf + BUFFOR_SIZE)
    {
        return 0;
    }

    return (size_t)(BUFFOR_SIZE + buf - p_buf);
}


void console_app_init(void)
{
	uint16_t err = console_driver_init();
	init_ok = (err == 0);

	if(!init_ok)
	{
		return;
	}
    // Draw Menu
    snprintf(p_buf, console_buf_left(), CONSOLE_CLEAR_SCREEN CONSOLE_CURSOR_HOME CONSOLE_RED"MENU\r\n"
        CONSOLE_WHITE "SCREEN 1: press 1");
    p_buf += strlen(p_buf);
    uint32_t len = p_buf - buf;
    err = console_driver_send(buf, len);
    p_buf = buf;
}

void console_app_step(void)
{
	if (!init_ok)
	{
		return;
	}

    uint8_t character = 0;
	uint16_t ret_val = console_driver_recv(&character);

	if (ret_val == CONSOLE_BAD_ARG)
	{
		return;
	}

	if (ret_val == 0)
	{
		screen_handle(character);
	}

if (console_screen != CONSOLE_MENU)
{
    redraw_step_cnt++;

    if (redraw_step_cnt >= CONSOLE_REDRAW_PERIOD_STEPS)
    {
        redraw_step_cnt = 0;
        redraw = true;
    }
}
else
{
    redraw_step_cnt = 0;
}

    if (redraw)
    {
    	screen_draw();
    }
}

static void screen_handle(uint8_t cmd)
{
    redraw = true;
    switch (console_screen)
    {
		case CONSOLE_MENU:
            switch (cmd)
            {
                case '1':
                    console_screen = CONSOLE_SCREEN1;
                    break;

                default:
                    redraw = false;
                    break;
            }
		break;

		case CONSOLE_SCREEN1:
            switch (cmd)
            {
                case '0':
                    console_screen = CONSOLE_MENU;
                    break;

                default:
                    redraw = false;
                    break;
            }

			break;

		case CONSOLE_ERROR:
			if(cmd == 'r')
			{
				console_screen = CONSOLE_MENU;
			}
			else
			{
				redraw = false;
			}
			break;

		default:
			console_screen = CONSOLE_ERROR;
            redraw = true;
            break;
	}
}

static void screen_draw(void)
{
	if (!console_driver_is_tx_ready())
	{
		return;
	}

	switch (console_screen)
	{
		case CONSOLE_MENU:
			snprintf(p_buf, console_buf_left(), CONSOLE_CLEAR_SCREEN CONSOLE_CURSOR_HOME CONSOLE_RED"MENU\r\n"
					CONSOLE_WHITE "SCREEN 1: press 1\r\n");
			p_buf+=strlen(p_buf);
			break;

		case CONSOLE_SCREEN1:
			snprintf(p_buf, console_buf_left(), CONSOLE_CLEAR_SCREEN CONSOLE_CURSOR_HOME CONSOLE_RED"EKRAN 1\r\n"
				CONSOLE_WHITE "MENU: press 0\r\n"
				"Temperatura %.2f C\r\n"
				"Cisnienie %.2f hPa",
				bmp280_get_temp_C(),
				bmp280_get_pres_hPa()
			);
			p_buf+=strlen(p_buf);
			break;

		default:
			snprintf(p_buf, console_buf_left(), CONSOLE_CLEAR_SCREEN CONSOLE_CURSOR_HOME CONSOLE_RED"ERROR\r\n"
				CONSOLE_WHITE "Return to MENU press r");
			p_buf+=strlen(p_buf);
			break;
		}

	uint32_t len = p_buf - buf;
	uint16_t err = console_driver_send(buf, len);
	if (err == 0)
	{
		redraw = false;
	}
	p_buf=buf;
}
