/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Serge Vakulenko
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "pico/unique_id.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "extern.h"

//
// Colors.
//
enum {
#ifdef VCCGND_YD_RP2040
    COLOR_RED    = 0x00070000, // red 3%
    COLOR_GREEN  = 0x03000000, // green 1%
    COLOR_BLUE   = 0x00000700, // blue 3%
#else
    COLOR_RED    = 0x0f000000, // red 6%
    COLOR_GREEN  = 0x000f0000, // green 6%
    COLOR_BLUE   = 0x00000f00, // blue 6%
#endif
    COLOR_BLACK  = 0x00000000, // black
    COLOR_WHITE  = 0x01010100, // white
};

// buffer to hold flash ID
char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

// true when got USB request
bool activity_flag;

int main(void)
{
    board_init();
    pico_get_unique_board_id_string(serial, sizeof(serial));

    //
    // A pin, to which WS2812 LED is connected,
    // is defined in the board description file.
    //
    const bool IS_RGBW = false;
    const unsigned SM = 0;
    const float FREQ = 800000;
    const unsigned offset = pio_add_program(pio0, &ws2812_program);

    ws2812_program_init(pio0, SM, offset, PICO_DEFAULT_WS2812_PIN, FREQ, IS_RGBW);

    unsigned prev_color = 0;
    pio_sm_put_blocking(pio0, 0, 0);
    sleep_ms(250);

    tud_init(BOARD_TUD_RHPORT);

    //TODO: add_repeating_timer_ms (int32_t delay_ms, repeating_timer_callback_t callback, void *user_data, repeating_timer_t *out)

    while (1) {
        // TinyUSB device task.
        tud_task();

        // LED color depends on USB state.
        unsigned led_color = prev_color;
        if (activity_flag) {
            led_color = COLOR_BLACK;
            activity_flag = false;
        } else if (tud_suspended()) {
            led_color = COLOR_BLUE;
        } else if (tud_mounted()) {
            led_color = COLOR_GREEN;
        } else if (tud_connected()) {
            led_color = COLOR_RED;
        } else {
            led_color = COLOR_WHITE;
        }

        // Update LED color.
        if (led_color != prev_color) {
            pio_sm_put_blocking(pio0, 0, led_color);
            prev_color = led_color;
        }
    }
}
