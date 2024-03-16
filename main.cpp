#include <stdio.h>
#include "pico/stdlib.h"

#define PICO_DEFAULT_LED_PIN 16

#ifndef PICO_DEFAULT_LED_PIN
#error Blinky example requires a board with a LED
#else
#define LED_PIN     PICO_DEFAULT_LED_PIN
#endif

#define MIN_DELAY_MS 100  // Minimum delay between interrupts
#define MAX_DELAY_MS 500  // Maximum delay between interrupts

int current_delay_ms = 250;  // Initial delay

bool led_callback(struct repeating_timer *t)
{
    static bool value = true;
    current_delay_ms = value ? 250 : 100;

    gpio_put(LED_PIN, value);
    value = !value;

    // Reset the timer with the updated delay
    cancel_repeating_timer(t);
    add_repeating_timer_ms(current_delay_ms, led_callback, NULL, t);

    return true;
}

int main()
{
    struct repeating_timer timer;

    stdio_init_all();

    printf("Hello from blinky example\n");
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    if(!add_repeating_timer_ms(current_delay_ms, led_callback, NULL, &timer)) {
        printf("Error: Failed to setup the timer");
        return 1;
    }

    while(1)
        tight_loop_contents();

    return 0;
}
