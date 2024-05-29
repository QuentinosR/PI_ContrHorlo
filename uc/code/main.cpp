#include "hw.h"
#include "ui.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#define NB_FLASHS 3
#define FLASH_ON_START_US 800
#define TRIG_OFF_MIN 4 //Otherwise alarm is set at a time already passed : Never executed
#define TRIG_PERIOD_START_US (1000000 / 8)
#define TRIG_ON_MIN_US(cameraMinExpTime) (cameraMinExpTime + 1) //+1 for safety
#define CORRECTION_US 3                                         // Correction for jitter
//<-tFlashOn->            <-tFlashOn->          <-tFlashOn->
//<-----tFlashPeriod----><-----tFlashPeriod---->
//____________           ____________           ____________                               ____
//|          |___________|          |___________|          |_______________________________|...

// <-----------------------tTrigOn------------------------>
//<---------------------------------------tTrigPeriod------------------------------------->
//__________________________________________________________                               ____
//|                                                        |_______________________________|...

//-- Exposure time --
//-- Trigger --
bool isTrigOffTmpUsed;
bool tTrigEnabled;
int tTrigOn;
int tTrigOff;
int tTrigMinExpTime;
volatile bool timTrigElapsedFlag;
volatile uint32_t timerHwTrigVal;

//-- Led flashs --
volatile bool timFlashElapsedFlag;
uint32_t flashTimes[NB_FLASHS][2]; //[Off time][On time]
volatile uint32_t timerHwFlashVal;

// Obligatory shared between cores
volatile trigger_cmd_t trigCmd;
volatile flash_cmd_t flashCmd;
volatile int cmdVal;
mutex_t mutexCmd;

// Messages
const char messErrorModTrig[] = "Impossible to modify trigger off time!";
const char messErrorModFlash[] = "Impossible to modify flash off time!";

int get_total_flash_duration()
{
    int cptUs = 0;
    for (int i = 0; i < NB_FLASHS; i++)
    {
        cptUs += flashTimes[i][0] + flashTimes[i][1];
    }
    return cptUs;
}

int trig_set_minimal_exposure_time(int expoTime)
{
    if (expoTime < 0)
        return -1;

    tTrigMinExpTime = expoTime + 1;
    return 0;
}

// Return -1 on illegal value
int trig_set_new_off_time(int newtOff)
{
    if (newtOff < TRIG_OFF_MIN)
        return -1;

    tTrigOff = newtOff;
    return 0;
}

// Has never to be called by user.
int trig_set_new_on_time(int newtOn)
{
    // On time has a minimum possible value
    if (newtOn < tTrigMinExpTime)
        return -1;

    int diffTime = newtOn - tTrigOn;

    int ec = trig_set_new_off_time(tTrigOff - diffTime); // Keep the period
    if( ec < 0)
        return -1;

    tTrigOn = newtOn;
    return 0;
}

// Three flashs is 3 ON + 2 OFF
int flashs_set_new_off_time(int newtOff)
{
    if (newtOff < 0)
        return -1;

    for (int i = 0; i < NB_FLASHS; i++)
    {
        flashTimes[i][0] = newtOff; // For the moment same value.
    }
    flashTimes[NB_FLASHS - 1][0] = 0;
    return 0;
}
int flashs_set_new_on_time(int newtOn)
{
    if (newtOn < 0)
        return -1;

    for (int i = 0; i < NB_FLASHS; i++)
    {
        flashTimes[i][1] = newtOn;
    }
    return 0;
}

// Only function that can be called on cmd
int flashs_and_trig_update(int newtOffLed, int newtOnLed)
{
    int ec;
    int prevtOffLed = flashTimes[0][0];
    int prevtOnLed = flashTimes[0][1];
    ec = flashs_set_new_off_time(newtOffLed);
    if (ec < 0)
        return -1;
    ec = flashs_set_new_on_time(newtOnLed);
    if (ec < 0)
        return -1;
    int totalFlashDuration = get_total_flash_duration();
    ec = trig_set_new_on_time(totalFlashDuration + CORRECTION_US); // Jitter is compensated by this correction
    //if error, set the previous ones
    if(ec < 0){
        //No possible error on these two functions because use previous good values
        flashs_set_new_off_time(prevtOffLed);
        flashs_set_new_on_time(prevtOnLed);
        return -1;
    }
    // printf("%d\n", totalFlashDuration);
    return 0;
}

static void timer_trig_callback(void)
{
    timerHwTrigVal = get_hw_timer_val();
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_TRIG_NUM);
    timTrigElapsedFlag = true;
}

static void timer_flash_callback(void)
{
    timerHwFlashVal = get_hw_timer_val();
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_FLASH_NUM);
    timFlashElapsedFlag = true;
}

int tTrigOffSave;
bool trigOutputState;
bool startFlash;
void trig_SM_process()
{
    int ec = 0xff;
    int cmdValSec = 0;
    queue_ui_in_type_t ui_log_type = LOG_NO_TYPE;

    if (timTrigElapsedFlag)
    {
        timTrigElapsedFlag = false;
        trigOutputState = !trigOutputState;

        // Beginning of high state -> Beginning of flash.
        if (trigOutputState)
        {
            startFlash = true;

            if (trigCmd != TRIG_NONE)
            { // Handle command
                bool mutexOwn = mutex_try_enter(&mutexCmd, NULL);
                if(mutexOwn){
                    cmdValSec = cmdVal;
                    switch (trigCmd)
                    {
                    case TRIG_ENABLE:
                        tTrigEnabled = cmdValSec == 1;
                        ec = 0;
                        ui_log_type = LOG_TRIG_ENABLE;
                        break;
                    case TRIG_OFF_TIME_SET:
                        
                        ec = cmdValSec = trig_set_new_off_time(cmdValSec);
                        ui_log_type = LOG_TRIG_OFF_TIME;
                        break;
                    case TRIG_OFF_TIME_SHIFT_SET:
                        isTrigOffTmpUsed = true;
                        tTrigOffSave = tTrigOff;
                        ec = trig_set_new_off_time(tTrigOff + cmdValSec);
                        ui_log_type = LOG_TRIG_OFF_TIME_SHIFT;
                        break;
                    case TRIG_EXPO_SET:
                        ec = trig_set_minimal_exposure_time(cmdValSec);
                        ui_log_type = LOG_TRIG_EXPO;
                        break;
                    default:
                        break;
                    }
                    trigCmd = TRIG_NONE; // Only for one action
                    mutex_exit(&mutexCmd);
                    if (ec == 0)
                    {
                        ui_enqueue_data_print((void *)&cmdValSec, ui_log_type);
                    }
                    if (ec == -1)
                    {
                        ui_enqueue_data_print((void *)messErrorModTrig, LOG_STRING);
                    }
                }
            }

            //<!> Handle command before setting trigger alarm.
            //<!> Otherwise, total flashs duration will be different than trigger.
            if (flashCmd != FLASH_NONE)
            {
                bool mutexOwn = mutex_try_enter(&mutexCmd, NULL);
                if(mutexOwn){
                    cmdValSec = cmdVal;
                    ec = 0xff;
                    switch (flashCmd)
                    {
                    case FLASH_ON_TIME_SET:
                        ec = flashs_and_trig_update(flashTimes[0][0], cmdValSec); // For the moment, all flashs have same period
                        ui_log_type = LOG_FLASH_ON_TIME;
                        break;
                    case FLASH_OFF_TIME_SET:
                        ec = flashs_and_trig_update(cmdValSec, flashTimes[0][1]); // For the moment, all flashs have same period
                        ui_log_type = LOG_FLASH_OFF_TIME;
                        break;
                    default:
                        break;
                    }
                    flashCmd = FLASH_NONE; // Only for one action
                    mutex_exit(&mutexCmd);

                    if (ec == 0)
                        ui_enqueue_data_print((void *)&cmdValSec, ui_log_type);
                    else if (ec == -1)
                        ui_enqueue_data_print((void *)messErrorModFlash, LOG_STRING);
                }
            }
        }

        int stateDuration = trigOutputState ? tTrigOn : tTrigOff;

        alarm_in_US(ALARM_TRIG_NUM, ALARM_TRIG_IRQ, timer_trig_callback, timerHwTrigVal, stateDuration);
        if (tTrigEnabled)
            gpio_put(TRIG_PIN, trigOutputState); // takes ~ 1.7us. Not a problem due to camera trigger delay

        if (isTrigOffTmpUsed && !trigOutputState)
        {
            isTrigOffTmpUsed = false;
            trig_set_new_off_time(tTrigOffSave); // Put the previous duration.
            ui_enqueue_data_print((void *)&tTrigOffSave, LOG_TRIG_OFF_TIME_SHIFT);
        }
    }
}

int iCurrFlash;
bool currFlashState;
void flash_SM_process()
{
    if (startFlash)
    {
        startFlash = false;
        iCurrFlash = -1; // Automatically increased to 0
        currFlashState = false;
        timer_flash_callback();
    }

    if (timFlashElapsedFlag)
    {
        timFlashElapsedFlag = false;
        currFlashState = !currFlashState;
        if (currFlashState) // One period done when we begin in On state.
            iCurrFlash++;

        int stateInt = currFlashState ? 1 : 0;

        // The last flash, has a off time of 0. So alarm setting function will automatically return.
        alarm_in_US(ALARM_FLASH_NUM, ALARM_FLASH_IRQ, timer_flash_callback, timerHwFlashVal, flashTimes[iCurrFlash][stateInt]);
        if (tTrigEnabled)
            gpio_put(LED_PIN, currFlashState);
    }
}

int init()
{
    int ec;

    hw_init();
    ui_init();
    mutex_init(&mutexCmd);

    // Flashs
    timFlashElapsedFlag = false;
    currFlashState = false; // Begin to false
    startFlash = false;
    flashCmd = FLASH_NONE;

    tTrigOn = 0; //<!> Don't modify it
    tTrigMinExpTime = 0;
    tTrigEnabled = false;

    // Trigger
    ec = trig_set_new_off_time(TRIG_PERIOD_START_US);
    if (ec != 0)
        return -1;

    // Currently, ON Time = OFF Time
    ec = flashs_and_trig_update(FLASH_ON_START_US, FLASH_ON_START_US);
    if (ec != 0)
        return -1;

    isTrigOffTmpUsed = false;
    trigOutputState = false; // Begin to false

    timTrigElapsedFlag = false;
    trigCmd = TRIG_NONE;

    timer_trig_callback();

    return 0;
}

int main()
{
    init(); // Common to both cores

    multicore_launch_core1(ui_task);

    queue_ui_in_entry_t ui_in_entry;
    const char test[] = "coucou!\n";

    ui_enqueue_data_print((void *)test, LOG_STRING);

    while (1)
    {
        trig_SM_process();
        flash_SM_process();
    }

    return 0;
}
