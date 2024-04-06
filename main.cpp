#include "hw.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#define NB_FLASHS 3

#define USE_TESTING_MODE 1
#define AUTO_START 1

#if USE_TESTING_MODE
#define CAMERA_EXPOSITION_TIME_US 1
#define FLASH_ON_US 5
#define TRIG_OFF_START_US 500
#define FLASH_OFF_STEP_US 100
#define TRIG_OFF_STEP_US 500
#else
#define CAMERA_EXPOSITION_TIME_US 19
#define FLASH_ON_US 10000
#define TRIG_OFF_START_US 100000
#define FLASH_OFF_STEP_US 10000
#define TRIG_OFF_STEP_US 50000
#endif

#define TRIG_ON_MIN_US (CAMERA_EXPOSITION_TIME_US + 1) //+1 for safety

#define CORRECTION_US 3

typedef enum trigger_cmd_t{
    TRIG_NONE = 0,
    TRIG_START,
    TRIG_STOP,
    TRIG_PERIOD_INC, //Increase the period by one step
    TRIG_PERIOD_DEC, //Decrease the period by one step
    TRIG_PERIOD_INC_ONE,  //Increase the period by one step for one period only
    TRIG_NB_CMDS
} trigger_cmd_t;

const char trigger_cmd_txt[TRIG_NB_CMDS]{
    'X',
    's',
    'e',
    'i',
    'd',
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
int tTrigOffSave;
bool isTrigOffTmpUsed;
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


//Queue definition
queue_t queue_ui_in;
queue_t queue_ui_out;
#define QUEUE_MESSAGE_SIZE 100
#define NB_QUEUE_ENTRIES 10
typedef enum queue_ui_in_type_t{
    MARCHE=0,
    TRIG_OFF_TIME,
    FLASH_OFF_TIME,
    STRING,
    NB_TYPES
} queue_ui_in_type_t;


typedef union queue_ui_in_data_t{
    double float64;
    uint32_t uint32;
    const char* str;
} queue_ui_in_data_t;

typedef struct 
{
    queue_ui_in_type_t type;
    queue_ui_in_data_t data;
} queue_ui_in_entry_t;

typedef struct 
{
    trigger_cmd_t cmd;
} queue_ui_out_entry_t;

//Messages
const char messErrorModTrig[] = "Impossible to modify trigger off time!";
const char messErrorModFlash[] = "Impossible to modify flash off time!";


void ui_enqueue_data(void* data, queue_ui_in_type_t type){
    queue_ui_in_entry_t ui_in_entry;
    ui_in_entry.type = type;

    switch(type){
        case MARCHE:
            ui_in_entry.data.float64 = *(double*)data;
            break;
        case TRIG_OFF_TIME:
        case FLASH_OFF_TIME:
            ui_in_entry.data.uint32 = *(uint32_t*)data;
            break;
        case STRING:
            ui_in_entry.data.str = (const char*) data;
            break;
    }
   
    queue_try_add(&queue_ui_in, &ui_in_entry);
}

int get_total_flash_duration(){
    int cptUs = 0;
    for(int i = 0; i < NB_FLASHS; i++){
        cptUs+= flashTimes[i][0] + flashTimes[i][1];
    }
    return cptUs;
}

//Return -1 on illegal value
int trig_set_new_off_time(int newtOff){
    if(newtOff < 0)
        return -1;

    tTrigOff = newtOff;
    return 0;
}

int trig_set_new_on_time(int newtOn){
    newtOn += CORRECTION_US;
    //On time has a minimum possible value
    if(newtOn < TRIG_ON_MIN_US)
        newtOn = TRIG_ON_MIN_US;

    int diffTime = newtOn - tTrigOn;

    tTrigOn = newtOn;

    return trig_set_new_off_time(tTrigOff - diffTime); //Keep the period

}

//Three flashs is 3 ON + 2 OFF
int flashs_set_new_off_time(int newtOff){
    if(newtOff < 0)
        return -1;

    for(int i = 0; i < NB_FLASHS; i++){
        flashTimes[i][0] = newtOff; //For the moment same value. 
        flashTimes[i][1] = FLASH_ON_US; // Doesn't change.
    }
    flashTimes[NB_FLASHS - 1][0] = 0;
    return 0;
}

//Only function that can be called on cmd
int flashs_and_trig_update(int newtOffLed){
    int ec;
    ec = flashs_set_new_off_time(newtOffLed);
    if(ec < 0) return -1;
    int totalFlashDuration = get_total_flash_duration();
    //printf("%d\n", totalFlashDuration);
    trig_set_new_on_time(totalFlashDuration);
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




//<-tFlashOn->            <-tFlashOn->          <-tFlashOn-> 
//<-----tFlashPeriod----><-----tFlashPeriod---->
//____________           ____________           ____________                               ____
//|          |___________|          |___________|          |_______________________________|...

// <-----------------------tTrigOn------------------------>
//<---------------------------------------tTrigPeriod------------------------------------->
//__________________________________________________________                               ____
//|                                                        |_______________________________|...


void trig_SM_process(){
    int ec = 0xff;

    if(trigCmd != TRIG_NONE){ //Handle command
        switch(trigCmd){
            case TRIG_START:
                timer_trig_callback();
                break;

            case TRIG_PERIOD_DEC:
                ec = trig_set_new_off_time(tTrigOff - TRIG_OFF_STEP_US);
                break;
            case TRIG_PERIOD_INC_ONE:
                isTrigOffTmpUsed = true;
                tTrigOffSave = tTrigOff;
            case TRIG_PERIOD_INC:
                ec = trig_set_new_off_time(tTrigOff + TRIG_OFF_STEP_US);
                break;


            default:
                break;
        }
        trigCmd = TRIG_NONE; //Only for one action
        if(ec == 0){
            ui_enqueue_data((void*)&tTrigOff, TRIG_OFF_TIME);
        }
        if(ec == -1){
            ui_enqueue_data((void*)messErrorModTrig, STRING);
        }
    }

    if(timTrigElapsedFlag){
        timTrigElapsedFlag = false; 
        trigOutputState = !trigOutputState;

        //Beginning of high state -> Beginning of flash.
        if(trigOutputState){
            startFlash = true;

            //<!> Handle command before setting trigger alarm.
            //<!> Otherwise, total flashs duration will be different than trigger. 
            if(flashCmd != LED_NONE){
                ec = 0xff;
                switch(flashCmd){
                    case LED_PERIOD_INC:
                        ec = flashs_and_trig_update(flashTimes[0][0] + FLASH_OFF_STEP_US);
                        break;

                    case LED_PERIOD_DEC:
                        ec = flashs_and_trig_update(flashTimes[0][0] - FLASH_OFF_STEP_US);
                        break;

                    default:
                        break;
                }
                flashCmd = LED_NONE; //Only for one action
                
                if(ec == 0){
                    ui_enqueue_data((void*)&flashTimes[0][0], FLASH_OFF_TIME);
                }
                if(ec == -1){
                    ui_enqueue_data((void*)messErrorModFlash, STRING);
                }
            }
        }

        int stateDuration = trigOutputState ? tTrigOn : tTrigOff;
       
        alarm_in_US(ALARM_TRIG_NUM, ALARM_TRIG_IRQ, timer_trig_callback, timerHwTrigVal, stateDuration);
        gpio_put(TRIG_PIN, trigOutputState); //takes ~ 1.7us. Not a problem due to camera trigger delay

        if(isTrigOffTmpUsed && !trigOutputState){
            isTrigOffTmpUsed = false;
            trig_set_new_off_time(tTrigOffSave); //Put the previous duration.
        }
    }

}

void flash_SM_process(){
    if(startFlash){
        startFlash = false;
        iCurrFlash = -1; //Automatically increased to 0
        currFlashState = false; 
        timer_flash_callback();
    }

    if(timFlashElapsedFlag){
        timFlashElapsedFlag = false;
        currFlashState = !currFlashState;
        if(currFlashState) //One period done when we begin in On state.
            iCurrFlash++;

        int stateInt = currFlashState ? 1 : 0;

        //The last flash, has a off time of 0. So alarm setting function will automatically return.
        alarm_in_US(ALARM_FLASH_NUM, ALARM_FLASH_IRQ, timer_flash_callback, timerHwFlashVal, flashTimes[iCurrFlash][stateInt]);
        gpio_put(LED_PIN, currFlashState);

    }
}

int init(){
    int ec;
    
    hw_init(cmd_handle);

    //Flashs
    timFlashElapsedFlag = false;
    currFlashState = false; //Begin to false
    startFlash = false;
    flashCmd = LED_NONE;

    tTrigOn = 0;

    //Trigger
    ec = trig_set_new_off_time(TRIG_OFF_START_US);
    if(ec != 0) return -1;

    ec = flashs_and_trig_update(FLASH_ON_US);
    if(ec != 0) return -1;

    isTrigOffTmpUsed = false;
    trigOutputState = false; //Begin to false

    timTrigElapsedFlag = false;
    trigCmd = AUTO_START ? TRIG_START : TRIG_NONE;

    return 0;

}

void process_core_1(){
    //Todo set UART interrupt to handle cmds

    queue_ui_in_entry_t entry;
    printf("hello form core 1\n");
    
    while(1){
        queue_remove_blocking(&queue_ui_in, &entry);
        if(entry.type == MARCHE){
            printf("[LOG] Marche : %f\n", entry.data.float64);
        }else if(entry.type == STRING){
            printf("[WARNING] %s\n", entry.data.str);
        }else if(entry.type == TRIG_OFF_TIME){
             printf("[LOG] New trig off time : %dus\n", entry.data.uint32);
        }else if(entry.type == FLASH_OFF_TIME){
             printf("[LOG] New flash off time : %dus\n", entry.data.uint32);
        }
    }

}

int main()
{
    init();
    queue_init(&queue_ui_in, sizeof(queue_ui_in_entry_t), NB_QUEUE_ENTRIES);
    queue_init(&queue_ui_out, sizeof(queue_ui_out_entry_t), NB_QUEUE_ENTRIES);

    multicore_launch_core1(process_core_1);

    queue_ui_in_entry_t ui_in_entry;
    const char test[] = "coucou!\n";

    ui_enqueue_data((void*)test, STRING);


    while(1){
        trig_SM_process();
        flash_SM_process();
        //queue_try_add(&queue_ui_in, &ui_in_entry);
    }

    return 0;
}
