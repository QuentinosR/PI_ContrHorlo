#include <stdio.h>
#include "pico/stdlib.h"


#define TRIG_PIN      16
#define LED_PIN       15
#define NB_LED_FLASHS 3


//-- Trigger --
int tTrigUs = 200;
int tTcUs = 20;
bool trigOutputState = false;
volatile bool timTrigElapsedFlag = false;
struct repeating_timer timerTrig;

//-- Led flashs --
int tLedUs = 10; 
int tFlashPeriod = 20;  //If tLi/i+1 = tLn/n+1
volatile bool timFlashElapsedFlag = false;
int ledTimes[NB_LED_FLASHS][2]; //[Off time][On time]
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

int init(){
    stdio_init_all();

    printf("Hello!\n");
    
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    for(int i = 0; i < NB_LED_FLASHS; i++){
        ledTimes[i][0] = tFlashPeriod - tLedUs; //For the moment same value. 
        ledTimes[i][1] = tLedUs;
    }

    if(!add_repeating_timer_us(1000, timer_trig_callback, NULL, &timerTrig)) { //arbitrary time
        printf("Error: Failed to setup the timerTrig for trigger");
        return 1;
    }
    return 0;
}

//Two states : OFF and ON
// <-tTcUs->
//<------------tTrigMS-------->
//__________                   __________
//|        |___________________|        |__...
void trig_SM_process(){
    if(timTrigElapsedFlag){
        timTrigElapsedFlag = false; //reset flag
        trigOutputState = !trigOutputState; //Begin by changing state
        if(trigOutputState)
            startFlash = true;
        int stateDuration = trigOutputState ? tTcUs : tTrigUs - tTcUs;
        // Reset the timerTrig with the updated delay
        cancel_repeating_timer(&timerTrig);
        add_repeating_timer_us(stateDuration, timer_trig_callback, NULL, &timerTrig);
        gpio_put(TRIG_PIN, trigOutputState);
    }
}

// <-tLed->             <-tLed->             <-tLed->
//<-------tL1/1-------><-------tL2/3------->
//<----------------------tFlash---------------------->
//__________           __________           __________
//|        |___________|        |___________|        |___...
void flash_SM_process(){
    if(startFlash){
        startFlash = false;
        iCurrLedFlash = -1;
        currLedFlashState = false; 
        timFlashElapsedFlag = true; //Simulate timer
    }

    if(timFlashElapsedFlag){
        timFlashElapsedFlag = false;
        currLedFlashState = !currLedFlashState;
        int stateInt = currLedFlashState ? 1 : 0;
        if(currLedFlashState){ //One period done when we begin in On state.
            iCurrLedFlash++;
            if(iCurrLedFlash >= NB_LED_FLASHS){
                cancel_repeating_timer(&timerFlash);
                return;
            }
        }

        cancel_repeating_timer(&timerFlash);
        add_repeating_timer_us(ledTimes[iCurrLedFlash][stateInt], timer_flash_callback, NULL, &timerFlash);
        gpio_put(LED_PIN, currLedFlashState);
    }
    //printf("blip!\n");
}

int main()
{
    init();
    while(1){
        //tight_loop_contents();
        trig_SM_process();
        flash_SM_process();
    }


    return 0;
}
