#include "stdlib.h"
#include "hw.h"
#include "ui.h"
#include <string.h>
#include "pico/multicore.h"
#include "pico/util/queue.h"

//Queue definition
//Impossible to have queue out to handle command because queue_remove takes too much time.
queue_t queue_ui_in;
extern trigger_cmd_t trigCmd;
extern flash_cmd_t flashCmd;
extern int cmdVal;

#define QUEUE_MESSAGE_SIZE 100
#define NB_QUEUE_ENTRIES 10

const char flash_cmd_txt[TRIG_NB_CMDS]{
    'X',
    'y',
    'z',
    'o',
};
const char trigger_cmd_txt[TRIG_NB_CMDS]{
    'X',
    's',
    'e',
    'i',
    'd',
    'r',

};


char cmdBuff[128];
int cmdBuffSize = 0;

void cmd_handle(char c){
    cmdBuff[cmdBuffSize++] = c;
    printf("%d\n", c);
    if(c != ';')
        return;
    
    cmdBuff[cmdBuffSize] = '\0';
    printf("new cmd ! %s\n", cmdBuff);

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
    //Check, can not exist : trig.start
    int val = atoi(pValue);
    cmdVal = val;
    cmdBuffSize = 0;
    printf("value : %s, %d\n", pValue, val);


    if(strcmp(components[0], "flash") == 0){

        if(strcmp(components[1], "on") == 0){
            printf("flash on !\n");
            flashCmd = LED_ON_TIME_SET;

        }else if(strcmp(components[1], "off") == 0){
            printf("flash off !\n");
            flashCmd = LED_OFF_TIME_SET;
        }

    }else if(strcmp(components[0], "trig") == 0){

        if(strcmp(components[1], "en") == 0){
            printf("trig en !\n");
        }
        else if(strcmp(components[1], "off") == 0){
            printf("trig off !\n");

        } else if(strcmp(components[1], "shift") == 0){
            printf("trig shift !\n");
        }
    }
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


void ui_task(){
    //Todo set UART interrupt to handle cmds
    init_uart_interrupt(cmd_handle);

    queue_ui_in_entry_t entry;
    printf("Hello form core 1\n");
    
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
