#include <stdio.h>
#include "pico/stdlib.h"


#define TRIG_PIN      16
#define LED_PIN       17
#define NB_LED_FLASHS 3


//-- Trigger --
int tTrigMs = 100;
int tTcMS = 1;
bool trigOutputState = false;
volatile bool timTrigElapsedFlag = false;
struct repeating_timer timerTrig;

//-- Led flashs --
int tLedMs = 2; 
int tFlashPeriod = 10;  //If tLi/i+1 = tLn/n+1
volatile bool timFlashElapsedFlag = false;
int ledTimes[NB_LED_FLASHS][2]; //[On time][Off time]
//   For state machine
int iCurrLedFlash = 0;
bool currLedFlashState = false;
bool startFlash = false;
struct repeating_timer timerFlash;

bool timer_trig_callback(struct repeating_timer *t)
{
    timTrigElapsedFlag = true;
    return true;
}

bool timer_flash_callback(struct repeating_timer *t){
    timFlashElapsedFlag = true;
    return true;
}

init init(){
    stdio_init_all();

    printf("Hello!\n");
    
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);

    for(int i = 0; i < NB_LED_FLASHS){
        ledTimes[i][0] = tLedMs;
        ledTimes[i][1] = tFlashPeriod - tLedMs; //For the moment same value. 
    }

    if(!add_repeating_timer_ms(1000, timer_trig_callback, NULL, &timerTrig)) { //arbitrary time
        printf("Error: Failed to setup the timerTrig for trigger");
        return 1;
    }
}

//Two states : OFF and ON
// <-tTcMS->
//<------------tTrigMS-------->
//__________                   __________
//|        |___________________|        |__...
void trig_SM_process(){
    if(timTrigElapsedFlag){
        timTrigElapsedFlag = false; //reset flag
        if(!trigOutputState)
            startFlash = 1;
        int stateDuration = trigOutputState ? tTcMS : tTrigMs - tTcMS;
        // Reset the timerTrig with the updated delay
        cancel_repeating_timer(&timerTrig);
        add_repeating_timer_ms(stateDuration, timer_trig_callback, NULL, &timerTrig);
        gpio_put(TRIG_PIN, trigOutputState);
        trigOutputState = !trigOutputState;
    }
}

void trigFlash_process(){
    if(startFlash){
        startFlash = false;
        iCurrLedFlash = 0;
        currLedFlashState = false;
        add_repeating_timer_ms(ledTimes[0][0], timer_flash_callback, NULL, &timerFlash);
    }
    if(timFlashElapsedFlag && iCurrLedFlash < NB_LED_FLASHS){
        timFlashElapsedFlag = false;
        if(!currLedFlashState) //Maybe oveflow ?
            iCurrLedFlash++;
        cancel_repeating_timer(&timerFlash);
        add_repeating_timer_ms(ledTimes[iCurrLedFlash][currLedFlashState], timer_flash_callback, NULL, &timerFlash);
        currLedFlashState = ! currLedFlashState;
    }
    //when finished, remove time.
}

int main()
{
    while(1)
        tight_loop_contents();
        trig_SM_process();
        trig_


    return 0;
}
