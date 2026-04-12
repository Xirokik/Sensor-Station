#include "display.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "encoder_driver.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

#define MENU_ITEMS_COUNT 4U
#define CONTRAST_MIN 16U
#define CONTRAST_MAX 255U

typedef enum
{
    DISPLAY_STATE_HOME = 0,
    DISPLAY_STATE_MENU,
    DISPLAY_STATE_TEMPERATURE,
    DISPLAY_STATE_HUMIDITY,
    DISPLAY_STATE_SETTINGS,
    DISPLAY_STATE_INFO
} display_state_t;

typedef enum
{
    MENU_ITEM_TEMPERATURE = 0,
    MENU_ITEM_HUMIDITY,
    MENU_ITEM_SETTINGS,
    MENU_ITEM_INFO
} menu_item_t;

typedef struct
{
    display_state_t state;
    uint32_t last_encoder_pos;
    uint8_t menu_index;
    uint8_t contrast;
    uint8_t refresh_requested;
    bool button_was_pressed;
} display_context_t;

static display_context_t display_ctx =
{
    .state = DISPLAY_STATE_HOME,
    .last_encoder_pos = 0U,
    .menu_index = 0U,
    .contrast = 0xCFU,
    .refresh_requested = 1U,
    .button_was_pressed = false
};

static const char *menu_items[MENU_ITEMS_COUNT] =
{
    "Temperature",
    "Humidity",
    "Settings",
    "Info"
};

static int32_t display_get_encoder_delta(uint32_t current_pos)
{
    int32_t delta = (int32_t)current_pos - (int32_t)display_ctx.last_encoder_pos;

    if (delta > ((int32_t)ENCODER_POSITIONS / 2))
    {
        delta -= (int32_t)ENCODER_POSITIONS;
    }
    else if (delta < -((int32_t)ENCODER_POSITIONS / 2))
    {
        delta += (int32_t)ENCODER_POSITIONS;
    }

    display_ctx.last_encoder_pos = current_pos;
    return delta;
}

static void draw_header(const char *title)
{
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString((char *)title, Font_7x10, White);
    ssd1306_Line(0, 12, 127, 12, White);
}

static void draw_home_screen(void)
{
    char line[24];

    draw_header("Sensor station");

    snprintf(line, sizeof(line), "Temp: %2u.%u C", 23U, 4U);
    ssd1306_SetCursor(0, 18);
    ssd1306_WriteString(line, Font_7x10, White);

    snprintf(line, sizeof(line), "Hum : %2u %%", 48U);
    ssd1306_SetCursor(0, 32);
    ssd1306_WriteString(line, Font_7x10, White);

    ssd1306_SetCursor(0, 50);
    ssd1306_WriteString("Press to open menu", Font_6x8, White);
}

static void draw_menu_screen(void)
{
    uint8_t i;

    draw_header("Main menu");

    for (i = 0U; i < MENU_ITEMS_COUNT; i++)
    {
        const uint8_t y = 16U + i * 12U;

        if (i == display_ctx.menu_index)
        {
            ssd1306_FillRectangle(0, y - 1U, 127, y + 9U, White);
            ssd1306_SetCursor(3, y);
            ssd1306_WriteString(">", Font_6x8, Black);
            ssd1306_SetCursor(14, y);
            ssd1306_WriteString((char *)menu_items[i], Font_6x8, Black);
        }
        else
        {
            ssd1306_SetCursor(14, y);
            ssd1306_WriteString((char *)menu_items[i], Font_6x8, White);
        }
    }
}

static void draw_temperature_screen(void)
{
    draw_header("Temperature");

    ssd1306_SetCursor(0, 20);
    ssd1306_WriteString("Current", Font_7x10, White);

    ssd1306_SetCursor(0, 28);
    ssd1306_WriteString("23.4 C", Font_11x18, White);

    ssd1306_SetCursor(0, 56);
    ssd1306_WriteString("Press to back", Font_6x8, White);
}

static void draw_humidity_screen(void)
{
    draw_header("Humidity");

    ssd1306_SetCursor(0, 20);
    ssd1306_WriteString("Current", Font_7x10, White);

    ssd1306_SetCursor(0, 28);
    ssd1306_WriteString("48 %", Font_11x18, White);

    ssd1306_SetCursor(0, 56);
    ssd1306_WriteString("Press to back", Font_6x8, White);
}

static void draw_settings_screen(void)
{
    char line[24];

    draw_header("Settings");

    snprintf(line, sizeof(line), "Contrast: %3u", display_ctx.contrast);
    ssd1306_SetCursor(0, 20);
    ssd1306_WriteString(line, Font_7x10, White);

    ssd1306_DrawRectangle(0, 38, 127, 48, White);
    ssd1306_FillRectangle(2, 40, (uint8_t)(2U + ((uint32_t)(display_ctx.contrast - CONTRAST_MIN) * 121U) /
        (CONTRAST_MAX - CONTRAST_MIN)), 46, White);

    ssd1306_SetCursor(0, 54);
    ssd1306_WriteString("Rotate: change  Press: back", Font_6x8, White);
}

static void draw_info_screen(void)
{
    draw_header("Info");

    ssd1306_SetCursor(0, 18);
    ssd1306_WriteString("Demo UI for encoder", Font_7x10, White);

    ssd1306_SetCursor(0, 32);
    ssd1306_WriteString("States: home/menu", Font_6x8, White);

    ssd1306_SetCursor(0, 42);
    ssd1306_WriteString("temp/hum/settings", Font_6x8, White);

    ssd1306_SetCursor(0, 54);
    ssd1306_WriteString("Press to back", Font_6x8, White);
}

static void display_render(void)
{
    ssd1306_Fill(Black);

    switch (display_ctx.state)
    {
        case DISPLAY_STATE_HOME:
            draw_home_screen();
            break;

        case DISPLAY_STATE_MENU:
            draw_menu_screen();
            break;

        case DISPLAY_STATE_TEMPERATURE:
            draw_temperature_screen();
            break;

        case DISPLAY_STATE_HUMIDITY:
            draw_humidity_screen();
            break;

        case DISPLAY_STATE_SETTINGS:
            draw_settings_screen();
            break;

        case DISPLAY_STATE_INFO:
            draw_info_screen();
            break;

        default:
            display_ctx.state = DISPLAY_STATE_HOME;
            draw_home_screen();
            break;
    }

    ssd1306_UpdateScreen();
    display_ctx.refresh_requested = 0U;
}

static void display_open_selected_menu_item(void)
{
    switch ((menu_item_t)display_ctx.menu_index)
    {
        case MENU_ITEM_TEMPERATURE:
            display_ctx.state = DISPLAY_STATE_TEMPERATURE;
            break;

        case MENU_ITEM_HUMIDITY:
            display_ctx.state = DISPLAY_STATE_HUMIDITY;
            break;

        case MENU_ITEM_SETTINGS:
            display_ctx.state = DISPLAY_STATE_SETTINGS;
            break;

        case MENU_ITEM_INFO:
        default:
            display_ctx.state = DISPLAY_STATE_INFO;
            break;
    }

    display_ctx.refresh_requested = 1U;
}

static void display_handle_rotation(int32_t encoder_delta)
{
    if (encoder_delta == 0)
    {
        return;
    }

    switch (display_ctx.state)
    {
        case DISPLAY_STATE_MENU:
        {
            int32_t next_index = (int32_t)display_ctx.menu_index + encoder_delta;

            while (next_index < 0)
            {
                next_index += MENU_ITEMS_COUNT;
            }

            display_ctx.menu_index = (uint8_t)(next_index % MENU_ITEMS_COUNT);
            display_ctx.refresh_requested = 1U;
            break;
        }

        case DISPLAY_STATE_SETTINGS:
        {
            int32_t next_contrast = (int32_t)display_ctx.contrast + encoder_delta * 8;

            if (next_contrast < CONTRAST_MIN)
            {
                next_contrast = CONTRAST_MIN;
            }
            else if (next_contrast > CONTRAST_MAX)
            {
                next_contrast = CONTRAST_MAX;
            }

            display_ctx.contrast = (uint8_t)next_contrast;
            ssd1306_SetContrast(display_ctx.contrast);
            display_ctx.refresh_requested = 1U;
            break;
        }

        default:
            break;
    }
}

static void display_handle_button(bool button_pressed)
{
    const bool button_clicked = (button_pressed && !display_ctx.button_was_pressed);

    display_ctx.button_was_pressed = button_pressed;

    if (!button_clicked)
    {
        return;
    }

    switch (display_ctx.state)
    {
        case DISPLAY_STATE_HOME:
            display_ctx.state = DISPLAY_STATE_MENU;
            break;

        case DISPLAY_STATE_MENU:
            display_open_selected_menu_item();
            return;

        case DISPLAY_STATE_TEMPERATURE:
        case DISPLAY_STATE_HUMIDITY:
        case DISPLAY_STATE_SETTINGS:
        case DISPLAY_STATE_INFO:
        default:
            display_ctx.state = DISPLAY_STATE_MENU;
            break;
    }

    display_ctx.refresh_requested = 1U;
}

void display_init(void)
{
    ssd1306_Init();
    ssd1306_SetContrast(display_ctx.contrast);
    display_ctx.last_encoder_pos = encoder_get_pos();
    display_ctx.refresh_requested = 1U;
}

void display_step(void)
{
    const uint32_t encoder_pos = encoder_get_pos();
    const int32_t encoder_delta = display_get_encoder_delta(encoder_pos);
    const bool button_pressed = is_encoder_button_pressed();

    display_handle_rotation(encoder_delta);
    display_handle_button(button_pressed);

    if (display_ctx.refresh_requested != 0U)
    {
        display_render();
    }
}