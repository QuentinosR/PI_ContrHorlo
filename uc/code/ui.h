#ifndef UI_H
#define UI_H

typedef enum trigger_cmd_t{
    TRIG_NONE = 0,
    TRIG_START,
    TRIG_STOP,
    TRIG_OFF_TIME,
    TRIG_OFF_TIME_SHIFT,  //Increase the period for one period only
    TRIG_NB_CMDS
} trigger_cmd_t;


typedef enum flash_cmd_t{
    FLASH_NONE = 0,
    FLASH_PERIOD_INC,
    FLASH_PERIOD_DEC,
    FLASH_OFF_TIME_SET,
    FLASH_ON_TIME_SET,
    FLASH_NB_CMDS

} flash_cmd_t;

typedef enum queue_ui_in_type_t{
    LOG_NO_TYPE = 0,
    LOG_MARCHE,
    LOG_TRIG_OFF_TIME,
    LOG_TRIG_OFF_TIME_SHIFT,
    LOG_FLASH_OFF_TIME,
    LOG_FLASH_ON_TIME,
    LOG_STRING,
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