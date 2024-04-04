#ifndef HW_H
#define HW_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

#define ALARM_TRIG_NUM 0
#define ALARM_TRIG_IRQ TIMER_IRQ_0
#define ALARM_FLASH_NUM 1
#define ALARM_FLASH_IRQ TIMER_IRQ_1

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define TRIG_PIN      16
#define LED_PIN       15

typedef void (*uart_callback_t)(char);

void alarm_in_US(uint8_t alarmNum, uint8_t alarmIrqId, irq_handler_t irqHandler, uint32_t timerHwIrq, uint32_t delay_US);
int hw_init(uart_callback_t cb);
uint32_t get_hw_timer_val();


#endif