#include "hw.h"
//TODO Not do last OFF period of flash.
//TODO Check if new flash value is authorized
//TODO Use SaveOffTime
#define NB_FLASHS 3

#define USE_TESTING_MODE 1
#define AUTO_START 0

#if USE_TESTING_MODE
#define CAMERA_EXPOSITION_TIME_US 1
#define FLASH_ON_US 5
#define TRIG_PERIOD_START_US 500
#else
#define CAMERA_EXPOSITION_TIME_US 19
#define FLASH_ON_US 1000
#define TRIG_PERIOD_START_US 20000
#endif

#define TRIG_ON_MIN_US (CAMERA_EXPOSITION_TIME_US + 1) //+1 for safety
#define FLASH_PERIOD_INIT_US (FLASH_ON_US * 2) //By default, duty cycle of 50%

#define GET_OFF_TIME(period, on) (period - on)
#define GET_ON_TIME(period, off) GET_OFF_TIME(period, off)

#define FLASH_PERIOD_STEP_US 100
#define TRIG_PERIOD_STEP_US 500

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
int tTrigPeriod;
int tTrigPeriodSave;
bool isTrigPeriodTmp;
int tTrigOn;
int tTrigOff;
bool trigOutputState;
volatile bool timTrigElapsedFlag;
volatile uint32_t timerHwTrigVal;
volatile trigger_cmd_t trigCmd;

//-- Led flashs --
volatile bool timFlashElapsedFlag;
uint32_t flashTimes[NB_FLASHS][2]; //[Off time][On time]
int iCurrFlash;
bool currFlashState;
bool startFlash;
volatile uint32_t timerHwFlashVal;
volatile flash_cmd_t flashCmd;

int get_total_flash_duration(){
    int cptUs = 0;
    for(int i = 0; i < NB_FLASHS; i++){
        cptUs+= flashTimes[i][0] + flashTimes[i][1];
    }
    return cptUs;
}

//Return -1 on illegal value
int trig_set_new_period(int newPeriod){
    int newtTrigOff = GET_OFF_TIME(newPeriod, tTrigOn);
    if(newtTrigOff < 0)
        return -1;

    tTrigOff = newtTrigOff;
    tTrigPeriod = newPeriod;
    return 0;
}
//On time has a minimum possible value
void trig_set_new_on_time(int newtOn){
    if(newtOn < TRIG_ON_MIN_US)
        newtOn = TRIG_ON_MIN_US;

    tTrigOn = newtOn;
}

//Three flashs is 3 ON + 2 OFF
int flashs_set_new_period(int newPeriod){
    int newtFlashOff = GET_OFF_TIME(newPeriod, flashTimes[0][1]); //All the same for the moment
    if(newtFlashOff < 0)
        return -1;

    for(int i = 0; i < NB_FLASHS; i++){
        flashTimes[i][0] = newtFlashOff; //For the moment same value. 
        flashTimes[i][1] = FLASH_PERIOD_INIT_US; // Doesn't change.
    }
    flashTimes[NB_FLASHS - 1][0] = 0;
    return 0;
}

int flashs_set_new_period_and_update_trig_time_on(int newPeriod){
    int ec;
    ec = flashs_set_new_period(newPeriod);
    if(ec < 0) return -1;
    int totalFlashDuration = get_total_flash_duration();
    trig_set_new_on_time(totalFlashDuration);
    trig_set_new_period(tTrigPeriod); //Set the same but adjust off time.
    return 0;
}


void cmd_handle(char c){
    for(uint8_t i = 0; i < TRIG_NB_CMDS; i++){
        if(trigger_cmd_txt[i] == c){
            trigCmd = (trigger_cmd_t) i;
            //printf("trigger cmd : %u\n", trigCmd);
            return;
        }
    }

    for(uint8_t i = 0; i < LED_NB_CMDS; i++){
        flash_cmd_t cmd = (flash_cmd_t) i;
        if(flash_cmd_txt[i] == c){
            flashCmd = (flash_cmd_t) i;
            //printf("flash cmd : %u\n", flashCmd);
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

void trig_SM_process(){
    static int ec = 0;
    if(trigCmd != TRIG_NONE){ //Handle command
        switch(trigCmd){
            case TRIG_START:
                timer_trig_callback();
                break;

            case TRIG_PERIOD_DEC_ONE:
                isTrigPeriodTmp = true;
                tTrigPeriodSave = tTrigPeriod;
            case TRIG_PERIOD_DEC:
                ec = trig_set_new_period(tTrigPeriod - TRIG_PERIOD_STEP_US);
                break;
            case TRIG_PERIOD_INC_ONE:
                isTrigPeriodTmp = true;
                tTrigPeriodSave = tTrigPeriod;
            case TRIG_PERIOD_INC:
                ec = trig_set_new_period(tTrigPeriod + TRIG_PERIOD_STEP_US);
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

//<-tFlashOn->            <-tFlashOn->          <-tFlashOn-> 
//<-----tFlashPeriod----><-----tFlashPeriod---->
//____________           ____________           ____________                               ____
//|          |___________|          |___________|          |_______________________________|...

// <-----------------------tTrigOn------------------------>
//<---------------------------------------tTrigPeriod------------------------------------->
//__________________________________________________________                               ____
//|                                                        |_______________________________|...
void flash_SM_process(){
    static int newFlashsPeriod;
    static int ec;

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
                newFlashsPeriod = flashTimes[0][0] + flashTimes[0][1] + FLASH_PERIOD_STEP_US;
                ec = flashs_set_new_period_and_update_trig_time_on(newFlashsPeriod);
                break;

            case LED_PERIOD_DEC:
                newFlashsPeriod = flashTimes[0][0] + flashTimes[0][1] - FLASH_PERIOD_STEP_US;
                ec = flashs_set_new_period_and_update_trig_time_on(newFlashsPeriod);
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

int init(){
    int ec;
    
    hw_init(cmd_handle);

    //Flashs
    timFlashElapsedFlag = false;
    currFlashState = false; //Begin to false
    startFlash = false;
    flashCmd = LED_NONE;

    //Trigger
    ec = flashs_set_new_period_and_update_trig_time_on(FLASH_PERIOD_INIT_US);
    if(ec != 0) return -1;
    ec = trig_set_new_period(TRIG_PERIOD_START_US);
    if(ec != 0) return -1;

    isTrigPeriodTmp = false;
    tTrigOff = GET_OFF_TIME(tTrigPeriod, tTrigOn);
    trigOutputState = false; //Begin to false

    timTrigElapsedFlag = false;
    trigCmd = AUTO_START ? TRIG_START : TRIG_NONE;

    return 0;

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
