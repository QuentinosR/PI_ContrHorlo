// Quentin Rod for PI - MSE Computer Science
#ifndef UI_H
#define UI_H

typedef enum trigger_cmd_t{
    TRIG_NONE = 0,
    TRIG_ENABLE,
    TRIG_OFF_TIME_SET,
    TRIG_OFF_TIME_SHIFT_SET,  //Increase the period for one period only
    TRIG_EXPO_SET,
    TRIG_NB_CMDS
} trigger_cmd_t;


typedef enum flash_cmd_t{
    FLASH_NONE = 0,
    FLASH_OFF_TIME_SET,
    FLASH_ON_TIME_SET,
    FLASH_NB_CMDS

} flash_cmd_t;

typedef enum queue_ui_in_type_t{
    LOG_NO_TYPE = 0,
    LOG_MARCHE,
    LOG_TRIG_ENABLE,
    LOG_TRIG_OFF_TIME,
    LOG_TRIG_OFF_TIME_SHIFT,
    LOG_TRIG_EXPO,
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