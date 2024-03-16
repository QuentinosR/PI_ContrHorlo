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


volatile int tTrigMs = 100;
volatile int tTcMS = 1;
volatile bool trigOutputState = false;

bool timer_trig_callback(struct repeating_timer *t)
{
    int stateDuration = trigOutputState ? tTcMS : tTrigMs - tTcMS;
    // Reset the timer with the updated delay
    cancel_repeating_timer(t);
    add_repeating_timer_ms(stateDuration, timer_trig_callback, NULL, t);
    gpio_put(LED_PIN, trigOutputState);
    trigOutputState = !trigOutputState;

    return true;
}

int main()
{
    struct repeating_timer timer;

    stdio_init_all();

    printf("Hello from blinky example\n");
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    if(!add_repeating_timer_ms(1000, timer_trig_callback, NULL, &timer)) { //arbitrary time
        printf("Error: Failed to setup the timer");
        return 1;
    }

    while(1)
        tight_loop_contents();

    return 0;
}
