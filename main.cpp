#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"

#define LED_DELAY_MS    250
#define PICO_DEFAULT_LED_PIN 16

#ifndef PICO_DEFAULT_LED_PIN
#error Blinky example requires a board with a LED
#else
#define LED_PIN     PICO_DEFAULT_LED_PIN
#endif

bool led_callback(struct repeating_timer *t)
{
    static bool value = true;

    gpio_put(LED_PIN, value);
    value = !value;

    return true;
}

int main()
{
    struct repeating_timer *timer = new struct repeating_timer;

    stdio_init_all();

    printf("Hello from blinky example\n");
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    if(!add_repeating_timer_ms(LED_DELAY_MS, led_callback, NULL, timer)) {
        printf("Error: Failed to setup the timer");
        return 1;
    }

    while(1)
        tight_loop_contents();

    return 0;
}