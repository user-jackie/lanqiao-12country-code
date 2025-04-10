#ifndef __USER_H
#define __USER_H

#include "main.h"
#include <stdio.h>
#include "tim.h"
#include "usart.h"
#include "adc.h"
#include "string.h"

extern u8 rx_data;

void led_disp(u8 led);
void led_proc(void);
void key_proc(void);
void lcd_proc(void);
void adc_proc(void);
void rx_proc(void);


#endif

