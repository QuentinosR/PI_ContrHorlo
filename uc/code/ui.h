#ifndef UI_H
#define UI_H

typedef enum trigger_cmd_t{
    TRIG_NONE = 0,
    TRIG_START,
    TRIG_STOP,
    TRIG_PERIOD_SET,
    TRIG_PERIOD_INC, //Increase the period by one step
    TRIG_PERIOD_DEC, //Decrease the period by one step
    TRIG_PERIOD_INC_ONE,  //Increase the period by one step for one period only
    TRIG_NB_CMDS
} trigger_cmd_t;


typedef enum flash_cmd_t{
    LED_NONE = 0,
    LED_PERIOD_INC,
    LED_PERIOD_DEC,
    LED_OFF_TIME_SET,
    LED_ON_TIME_SET,
    LED_NB_CMDS

} flash_cmd_t;

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

void ui_init();
void ui_task();
void ui_enqueue_data_print(void* data, queue_ui_in_type_t type);

#endif