#include "hw.h"
#define NB_FLASHS 3

#define USE_TESTING_MODE 0
#define AUTO_START 0

#if USE_TESTING_MODE
#define TRIG_PERIOD_US 100
#define TRIG_ON_US 20
#define FLASH_ON_US 5
#define FLASH_PERIOD_US 10
#else
#define TRIG_PERIOD_US 10000
#define TRIG_ON_US 5000
#define FLASH_ON_US 1000
#define FLASH_PERIOD_US 2000
#endif

#define GET_OFF_TIME(period, on) (period - on)
#define FLASH_PERIOD_STEP_US (FLASH_PERIOD_US / 10)
#define TRIG_PERIOD_STEP_US (TRIG_PERIOD_US / 10)

typedef enum trigger_cmd_t{
    TRIG_NONE = 0,
    TRIG_START,
    TRIG_STOP,
    TRIG_PERIOD_INC, //Increase the period by one step
    TRIG_PERIOD_DEC, //Decrease the period by one step
    TRIG_PERIOD_DEC_ONE,  //Decrease the period by one step for one period only
    TRIG_PERIOD_INC_ONE,  //Increase the period by one step for one period only
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

//-- Trigger --
int tTrigPeriod = TRIG_PERIOD_US;
int tTrigPeriodSave = tTrigPeriod;
bool isTrigPeriodTmp = false;
int tTrigOn = TRIG_ON_US; //Can not be modified.
int tTrigOff = GET_OFF_TIME(tTrigPeriod, tTrigOn);
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

//Return -1 on illegal value
int trig_set_new_period(int newPeriod){

    int newtTrigOff = GET_OFF_TIME(newPeriod, tTrigOn);
    if(newtTrigOff < 0)
        return -1;

    tTrigOff = newtTrigOff;
    tTrigPeriod = newPeriod;
    return 0;
}


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



//For the moment same value for On time
void update_flash_times_array(){
    for(int i = 0; i < NB_FLASHS; i++){
        flashTimes[i][0] = GET_OFF_TIME(tFlashPeriod, tFlashOn); //For the moment same value. 
    }
}


//Two states : OFF and ON
// <-tTrigOn->
//<------------tTrigMS-------->
//__________                   __________
//|        |___________________|        |__...
void trig_SM_process(){
    int errCode = 0;
    if(trigCmd != TRIG_NONE){ //Command to handle
        switch(trigCmd){
            case TRIG_START:
                //TODO : Modify tTrigOn, Has to be off + on for each at startup.
                //Has to be a minimum
                timer_trig_callback();
                break;

            case TRIG_PERIOD_DEC_ONE:
                isTrigPeriodTmp = true;
                tTrigPeriodSave = tTrigPeriod;
            case TRIG_PERIOD_DEC:
                errCode = trig_set_new_period(tTrigPeriod - TRIG_PERIOD_STEP_US);
                break;
            case TRIG_PERIOD_INC_ONE:
                isTrigPeriodTmp = true;
                tTrigPeriodSave = tTrigPeriod;
            case TRIG_PERIOD_INC:
                errCode = trig_set_new_period(tTrigPeriod + TRIG_PERIOD_STEP_US);
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

        //To modify the period, off time is adjusted
        if(isTrigPeriodTmp && !trigOutputState){
            isTrigPeriodTmp = false;
            trig_set_new_period(tTrigPeriodSave);
        }
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
        iCurrFlash = -1;
        currFlashState = false; 
        timer_flash_callback();
    }

    //Command has to be handled only when flash begin.
    //When -1, it's obligatory starting.
    if(flashCmd != LED_NONE && iCurrFlash == -1){
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

void init(){
    hw_init(cmd_handle);
    for(int i = 0; i < NB_FLASHS; i++){
        flashTimes[i][0] = GET_OFF_TIME(tFlashPeriod, tFlashOn); //For the moment same value. 
        flashTimes[i][1] = tFlashOn;
    }

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
