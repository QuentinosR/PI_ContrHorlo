#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

#define ALARM_TRIG_NUM 0
#define ALARM_TRIG_IRQ TIMER_IRQ_0
#define ALARM_FLASH_NUM 1
#define ALARM_FLASH_IRQ TIMER_IRQ_1

#define TRIG_PIN      16
#define LED_PIN       15
#define NB_LED_FLASHS 3

static uint32_t get_hw_timer_val(){
    return timer_hw->timerawl;
}

//-- Trigger --
int tTrigUs = 100;
int tTcUs = 20;
int tTrigOn = tTcUs;
int tTrigOff = tTrigUs - tTcUs;
bool trigOutputState = false;
volatile bool timTrigElapsedFlag = false;
volatile uint32_t timerHwTrigVal = get_hw_timer_val();

//-- Led flashs --
int tLedUs = 10; 
int tFlashPeriod = 20;  //If tLi/i+1 = tLn/n+1
volatile bool timFlashElapsedFlag = false;
uint32_t ledTimes[NB_LED_FLASHS][2]; //[Off time][On time]
//   For state machine
int iCurrLedFlash = 0;
bool currLedFlashState = false;
bool startFlash = false;
volatile uint32_t timerHwFlashVal = 0;

static void timer_trig_callback(void) {
    timerHwTrigVal = get_hw_timer_val();
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_TRIG_NUM);
    timTrigElapsedFlag = true;
}

static void timer_flash_callback(void){
    timerHwFlashVal = get_hw_timer_val();
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_FLASH_NUM);
    timFlashElapsedFlag = true;
}

static void alarm_in_us(uint8_t alarmNum, uint8_t alarmIrqId, irq_handler_t irqHandler, uint32_t timerHwIrq, uint32_t delay_us) {
    // Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
    hw_set_bits(&timer_hw->inte, 1u << alarmNum);
    // Set irq handler for alarm irq
    irq_set_exclusive_handler(alarmIrqId, irqHandler);
    // Enable the alarm irq
    irq_set_enabled(alarmIrqId, true);
    // Enable interrupt in block and at processor

    // Alarm is only 32 bits so if trying to delay more
    // than that need to be careful and keep track of the upper
    // bits
    uint64_t target = timerHwIrq + delay_us;

    // Write the lower 32 bits of the target time to the alarm which
    // will arm it
    timer_hw->alarm[alarmNum] = (uint32_t) target;
}


int init(){
    stdio_init_all();
    
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    for(int i = 0; i < NB_LED_FLASHS; i++){
        ledTimes[i][0] = tFlashPeriod - tLedUs; //For the moment same value. 
        ledTimes[i][1] = tLedUs;
    }

    alarm_in_us(ALARM_TRIG_NUM, ALARM_TRIG_IRQ, timer_trig_callback,timerHwTrigVal, 1000);
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
        int stateDuration = trigOutputState ? tTrigOn : tTrigOff;
        // Reset the timerTrig with the updated delay
        alarm_in_us(ALARM_TRIG_NUM, ALARM_TRIG_IRQ, timer_trig_callback, timerHwTrigVal, stateDuration);
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
        timerHwFlashVal = get_hw_timer_val();
    }

    if(timFlashElapsedFlag){
        timFlashElapsedFlag = false;
        currLedFlashState = !currLedFlashState;
        int stateInt = currLedFlashState ? 1 : 0;
        if(currLedFlashState){ //One period done when we begin in On state.
            iCurrLedFlash++;
            if(iCurrLedFlash >= NB_LED_FLASHS){
                return;
            }
        }

        alarm_in_us(ALARM_FLASH_NUM, ALARM_FLASH_IRQ, timer_flash_callback, timerHwFlashVal, ledTimes[iCurrLedFlash][stateInt]);
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
