#include "hw.h"
#include "ui.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

//Queue definition
//Impossible to have queue out to handle command because queue_remove takes too much time.
queue_t queue_ui_in;
extern trigger_cmd_t trigCmd;
extern flash_cmd_t flashCmd;

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

void ui_init(){
    queue_init(&queue_ui_in, sizeof(queue_ui_in_entry_t), NB_QUEUE_ENTRIES);
}

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
