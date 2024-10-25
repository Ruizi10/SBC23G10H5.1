#ifndef __ADC_READ__
#define __ADC_READ__

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "ssd1306.h"
#include "font8x8_basic.h"
#include "esp_adc/adc_continuous.h"
#include "driver/gpio.h"


#ifdef __cplusplus
extern "C"
{
#endif

/*
 You have to set this config value with menuconfig
 CONFIG_INTERFACE

 for i2c
 CONFIG_MODEL
 CONFIG_SDA_GPIO
 CONFIG_SCL_GPIO
 CONFIG_RESET_GPIO

 for SPI
 CONFIG_CS_GPIO
 CONFIG_DC_GPIO
 CONFIG_RESET_GPIO
*/

/**
 * Link del video:
 * 
 * https://drive.google.com/drive/folders/1-DIC5Aa1Ngz1XokyykyCuvCJeiOuh48o?usp=drive_link
 * 
 * */


#define PACK8 __attribute__((aligned( __alignof__( uint8_t ) ), packed ))
typedef union out_column_t {
	uint32_t u32;
	uint8_t  u8[4];
} PACK8 out_column_t;

#define tag "SSD1306"

//static const char *TAG_ADC_READ_LOOP = "ADC_READ_LOOP";
#define BLINK_GPIO CONFIG_BLINK_GPIO

#define CONFIG_BLINK_LED_GPIO 1

#ifdef CONFIG_BLINK_LED_GPIO


#define EXAMPLE_ADC_UNIT                    ADC_UNIT_1
#define _EXAMPLE_ADC_UNIT_STR(unit)         #unit
#define EXAMPLE_ADC_UNIT_STR(unit)          _EXAMPLE_ADC_UNIT_STR(unit)
#define EXAMPLE_ADC_CONV_MODE               ADC_CONV_SINGLE_UNIT_1
#define EXAMPLE_ADC_ATTEN                   ADC_ATTEN_DB_0
#define EXAMPLE_ADC_BIT_WIDTH               SOC_ADC_DIGI_MAX_BITWIDTH

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define EXAMPLE_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define EXAMPLE_ADC_GET_CHANNEL(p_data)     ((p_data)->type1.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)        ((p_data)->type1.data)
#else
#define EXAMPLE_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define EXAMPLE_ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)        ((p_data)->type2.data)
#endif

#define EXAMPLE_READ_LEN                    256

#endif

#define DATA_INPUT_CH 6

/**
 * Salida de canales 6 / 7:
 * 
 * CH 6: 2-> 6-> 10 ... ((2) .. SUM += 4)
 * CH 7: 0-> 4-> 8 ... ((0) .. SUM += 4)
 */

#ifdef DATA_INPUT_CH

#if DATA_INPUT_CH == 6
#define DATA_READ_INIT 2
#else
#define DATA_READ_INIT 0
#endif

#else
#error "Undefined DATA_INPUT_CH"
#endif

/*

Estaticos solo en fuente .c

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t, const adc_continuous_evt_data_t *, void *);
static void continuous_adc_init(adc_channel_t *, uint8_t, adc_continuous_handle_t *);

*/

// Modificacion de ssd1306_display_text_x3 a x2

void ssd1306_display_text_x2(SSD1306_t *, int, const char *, int, bool);
// Layout personalizado oled
void disp_upd(SSD1306_t *, uint16_t, const uint16_t);
void read_main(void);
float_t get_average_single_read(void);


#ifdef __cplusplus
}
#endif

#endif /* !__ADC_READ__ */