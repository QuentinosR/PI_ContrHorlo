#include "stdlib.h"
#include "hw.h"
#include "ui.h"
#include <string.h>
#include "pico/multicore.h"
#include "pico/util/queue.h"

queue_t queue_ui_in;
//extern is not the cleanest way. Unfortunatly, using a queue to handle commands will delay a lot
//the signal generation.
extern trigger_cmd_t trigCmd;
extern flash_cmd_t flashCmd;
extern int cmdVal;
extern mutex_t mutexCmd;

#define QUEUE_MESSAGE_SIZE 100
#define NB_QUEUE_ENTRIES 10


char cmdBuff[128];
int cmdBuffSize = 0;

void cmd_handle(char c){
    cmdBuff[cmdBuffSize++] = c;
    //printf("%d\n", c);
    if(c != ';')
        return;
    
    cmdBuff[cmdBuffSize] = '\0';
    //printf("[CMD] new cmd : %s\n", cmdBuff);

    char *components[4]; // Array to hold the parsed components
    int index = 0;

    char* pEndHeader = strchr(cmdBuff, ':');
    if(pEndHeader == NULL){
        cmdBuffSize = 0;
        return;
    }
    *pEndHeader = '\0';
   
    char *sub_token = strtok(cmdBuff, ".");
    while (sub_token != NULL) {
        components[index++] = sub_token;
        sub_token = strtok(NULL, ".");
    }

    char* pValue = strtok(pEndHeader + 1, ";");
    if(pValue == NULL){
        cmdBuffSize = 0;
        return;
    }
   
    /*for(int i = 0; i < index; i++){
        printf("sub element : %s\n", components[i]);
    }*/
    int val = atoi(pValue);
    mutex_enter_blocking(&mutexCmd);
    cmdVal = val;
    //printf("value : %s, %d\n", pValue, val);

    if(strcmp(components[0], "flash") == 0){

        if(strcmp(components[1], "on") == 0){
            //printf("[CMD] set flash on\n");
            flashCmd = FLASH_ON_TIME_SET;

        }else if(strcmp(components[1], "off") == 0){
            //printf("[CMD] set flash off\n");
            flashCmd = FLASH_OFF_TIME_SET;
        }

    }else if(strcmp(components[0], "trig") == 0){

        if(strcmp(components[1], "en") == 0){
            
            trigCmd = TRIG_ENABLE;
            //printf("[CMD] set trig enable\n");
        }
        else if(strcmp(components[1], "off") == 0){
            //printf("[CMD] set trig off\n");
            trigCmd = TRIG_OFF_TIME_SET;

        } else if(strcmp(components[1], "shift") == 0){
            //printf("[CMD] set trig shift\n");
            trigCmd = TRIG_OFF_TIME_SHIFT_SET;
        }else if(strcmp(components[1], "expo") == 0){
            //printf("[CMD] set trig minimal exposure time\n");
            trigCmd = TRIG_EXPO_SET;
        }
    }
    mutex_exit(&mutexCmd);
    cmdBuffSize = 0;
    return;
    //uart_putc(UART_ID, c);
}

void ui_init(){
    queue_init(&queue_ui_in, sizeof(queue_ui_in_entry_t), NB_QUEUE_ENTRIES);
}

//It's safer to copy data in order to avoid concurrency issues.
//string has to be const.
void ui_enqueue_data_print(void* data, queue_ui_in_type_t type){
    queue_ui_in_entry_t ui_in_entry;
    ui_in_entry.type = type;

    switch(type){
        case LOG_MARCHE:
            ui_in_entry.data.float64 = *(double*)data;
            break;
        case LOG_TRIG_ENABLE:
        case LOG_TRIG_OFF_TIME:
        case LOG_TRIG_OFF_TIME_SHIFT:
        case LOG_FLASH_ON_TIME:
        case LOG_FLASH_OFF_TIME:
        case LOG_TRIG_EXPO:
            ui_in_entry.data.uint32 = *(uint32_t*)data;
            break;
        case LOG_STRING:
            ui_in_entry.data.str = (const char*) data;
            break;
    }
   
    queue_try_add(&queue_ui_in, &ui_in_entry);
}


void ui_task(){
    //Todo set UART interrupt to handle cmds
    init_uart_interrupt(cmd_handle);

    queue_ui_in_entry_t entry;
    
    while(1){
        queue_remove_blocking(&queue_ui_in, &entry);
        if(entry.type == LOG_MARCHE){
            printf("[LOG] Marche : %f\n", entry.data.float64);
        }else if(entry.type == LOG_STRING){
            printf("[WARNING] %s\n", entry.data.str);
        }else if(entry.type == LOG_TRIG_ENABLE){
             printf("[LOG] Trig state : %s\n", entry.data.uint32 == 1 ? "On" : "Off");
        }else if(entry.type == LOG_TRIG_OFF_TIME){
             printf("[LOG] New trig off time : %dus\n", entry.data.uint32);
        }
        else if(entry.type == LOG_TRIG_OFF_TIME_SHIFT){
             printf("[LOG] Shift done for one period only : %dus\n", entry.data.uint32);
        }else if(entry.type == LOG_TRIG_EXPO){
             printf("[LOG] New minimum exposure time : %dus\n", entry.data.uint32);
        }else if(entry.type == LOG_FLASH_OFF_TIME){
             printf("[LOG] New flash off time : %dus\n", entry.data.uint32);
        }else if(entry.type == LOG_FLASH_ON_TIME){
             printf("[LOG] New flash on time : %dus\n", entry.data.uint32);
        }
    }

}
