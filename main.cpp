#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

#define ALARM_TRIG_NUM 0
#define ALARM_TRIG_IRQ TIMER_IRQ_0
#define ALARM_FLASH_NUM 1
#define ALARM_FLASH_IRQ TIMER_IRQ_1

#define TRIG_PIN      16
#define LED_PIN       15
#define NB_FLASHS 3

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define USE_TESTING_MODE 1
#define AUTO_START 0

#if USE_TESTING_MODE
#define TRIG_PERIOD_US 100
#define TRIG_ON_US 20
#define FLASH_ON_US 5
#define FLASH_PERIOD_US 10
#else
#define TRIG_PERIOD_US 100000
#define TRIG_ON_US 2000
#define FLASH_ON_US 1000
#define FLASH_PERIOD_US 10000
#endif

#define FLASH_PERIOD_STEP_US (FLASH_PERIOD_US / 10)
#define TRIG_PERIOD_STEP_US (TRIG_PERIOD_US / 10)

typedef enum trigger_cmd_t{
    TRIG_NONE = 0,
    TRIG_START,
    TRIG_STOP,
    TRIG_PERIOD_INC, //Increase the period by one step
    TRIG_PERIOD_DEC, //Decrease the period by one step
    TRIG_PERIOD_DEC_ONE,  //Decrease the period by one step for one period only
    TRIG_PERIODE_INC_ONE,  //Increase the period by one step for one period only
    TRIG_NB_CMDS

} trigger_cmd_t;

const char trigger_cmd_txt[TRIG_NB_CMDS]{
    'X',
    's',
    'e',
    'i',
    'd',
    'a',
    'r',

};

typedef enum flash_cmd_t{
    LED_NONE = 0,
    LED_PERIOD_INC,
    LED_PERIOD_DEC,
    LED_NB_CMDS

} flash_cmd_t;


const char flash_cmd_txt[TRIG_NB_CMDS]{
    'X',
    'y',
    'z',
    'o',
};

static uint32_t get_hw_timer_val(){
    return timer_hw->timerawl;
}

//-- Trigger --
int tTrigPeriod = TRIG_PERIOD_US;
int tTrigOn = TRIG_ON_US; //Can not be modified
int tTrigOff = tTrigPeriod - tTrigOn;
bool trigOutputState = false;
volatile bool timTrigElapsedFlag = false;
volatile uint32_t timerHwTrigVal = get_hw_timer_val();
volatile trigger_cmd_t trigCmd = AUTO_START ? TRIG_START : TRIG_NONE;

//-- Led flashs --
int tFlashOn = FLASH_ON_US; //Can not be modified
int tFlashPeriod = FLASH_PERIOD_US;  //If tLi/i+1 = tLn/n+1
volatile bool timFlashElapsedFlag = false;
uint32_t flashTimes[NB_FLASHS][2]; //[Off time][On time]
//   For state machine
int iCurrFlash = 0;
bool currFlashState = false;
bool startFlash = false;
volatile uint32_t timerHwFlashVal = 0;
volatile flash_cmd_t flashCmd = LED_NONE;


void cmd_handle(char c){
    for(uint8_t i = 0; i < TRIG_NB_CMDS; i++){
        if(trigger_cmd_txt[i] == c){
            trigCmd = (trigger_cmd_t) i;
            printf("trigger cmd : %u\n", trigCmd);
            return;
        }
    }

    for(uint8_t i = 0; i < LED_NB_CMDS; i++){
        flash_cmd_t cmd = (flash_cmd_t) i;
        if(flash_cmd_txt[i] == c){
            flashCmd = (flash_cmd_t) i;
            printf("flash cmd : %u\n", flashCmd);
            return;
        }
    }
    //uart_putc(UART_ID, c);
}

// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t c = uart_getc(UART_ID);
        // Can we send it back?
        if (uart_is_writable(UART_ID)) {
            cmd_handle(c);
            //uart_putc(UART_ID, c);
        }
    }
}

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

static void alarm_in_US(uint8_t alarmNum, uint8_t alarmIrqId, irq_handler_t irqHandler, uint32_t timerHwIrq, uint32_t delay_US) {
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
    uint64_t target = timerHwIrq + delay_US;

    // Write the lower 32 bits of the target time to the alarm which
    // will arm it
    timer_hw->alarm[alarmNum] = (uint32_t) target;
}

void init_uart(){
      // Set up our UART with a basic baud rate.
    uart_init(UART_ID, 2400);

    // Set the TX and RX pins by USing the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);
 
    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are USing
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!
    //uart_puts(UART_ID, "\nHello, uart interrupts\n");
}

//For the moment same value
void update_flash_times_array(){
    for(int i = 0; i < NB_FLASHS; i++){
        flashTimes[i][0] = tFlashPeriod - tFlashOn; //For the moment same value. 
    }
}

int init(){
    stdio_init_all();
    
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_put(TRIG_PIN, 0);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
    printf("begin\n");

    init_uart();
    for(int i = 0; i < NB_FLASHS; i++){
        flashTimes[i][0] = tFlashPeriod - tFlashOn; //For the moment same value. 
        flashTimes[i][1] = tFlashOn;
    }

    return 0;
}

//Two states : OFF and ON
// <-tTrigOn->
//<------------tTrigMS-------->
//__________                   __________
//|        |___________________|        |__...
void trig_SM_process(){
    if(trigCmd != TRIG_NONE){ //Command to handle
        switch(trigCmd){
            case TRIG_START:
                timer_trig_callback();
                break;
            case TRIG_PERIOD_INC:
                tTrigPeriod += TRIG_PERIOD_STEP_US;
                tTrigOff = tTrigPeriod - tTrigOn;
                
                break;
            case TRIG_PERIOD_DEC:
                tTrigPeriod -= TRIG_PERIOD_STEP_US;
                tTrigOff = tTrigPeriod - tTrigOn;
                break;
            default:
                break;
        }
        trigCmd = TRIG_NONE; //Cmd only for one action

    }

    if(timTrigElapsedFlag){
        timTrigElapsedFlag = false; //reset flag
        trigOutputState = !trigOutputState; //Begin by changing state
        if(trigOutputState)
            startFlash = true;
        int stateDuration = trigOutputState ? tTrigOn : tTrigOff;
        // Reset the timerTrig with the updated delay
        alarm_in_US(ALARM_TRIG_NUM, ALARM_TRIG_IRQ, timer_trig_callback, timerHwTrigVal, stateDuration);
        gpio_put(TRIG_PIN, trigOutputState);
    }
}

// <-tLed->             <-tLed->             <-tLed->
//<-------tL1/1-------><-------tL2/3------->
//<----------------------tFlash---------------------->
//__________           __________           __________
//|        |___________|        |___________|        |___...
void flash_SM_process(){
    if(flashCmd != LED_NONE){
        switch(flashCmd){
            case LED_PERIOD_INC:
                tFlashPeriod += FLASH_PERIOD_STEP_US;
                update_flash_times_array();
                break;
            case LED_PERIOD_DEC:
                tFlashPeriod -= FLASH_PERIOD_STEP_US;
                update_flash_times_array();
                break;
            default:
                break;
        }
        flashCmd = LED_NONE;
    }
    if(startFlash){
        startFlash = false;
        iCurrFlash = -1;
        currFlashState = false; 
        timer_flash_callback();
    }

    if(timFlashElapsedFlag){
        timFlashElapsedFlag = false;
        currFlashState = !currFlashState;
        int stateInt = currFlashState ? 1 : 0;
        if(currFlashState){ //One period done when we begin in On state.
            iCurrFlash++;
            if(iCurrFlash >= NB_FLASHS){
                return;
            }
        }

        alarm_in_US(ALARM_FLASH_NUM, ALARM_FLASH_IRQ, timer_flash_callback, timerHwFlashVal, flashTimes[iCurrFlash][stateInt]);
        gpio_put(LED_PIN, currFlashState);

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
