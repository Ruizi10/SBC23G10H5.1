#include "adc_read.h"


static const char *TAG_ADC_READ_LOOP = "ADC_READ_LOOP";

#if CONFIG_IDF_TARGET_ESP32
static adc_channel_t channel[2] = {ADC_CHANNEL_6, ADC_CHANNEL_7};
#else
static adc_channel_t channel[2] = {ADC_CHANNEL_2, ADC_CHANNEL_3};
#endif

static TaskHandle_t s_task_handle;

/**
 * Formato salida oled:
 * 
 *  |:: Lectura    ::|
 *  |>    0596     < |
 *  |:: Intensidad ::|
 *  |           0    |
 *  |__minima________|
 * 
 *  |:: Lectura    ::|
 *  |>    3080     < |
 *  |:: Intensidad ::|
 *  |  | | | |  4    |
 *  |__alta__________|
 * 
 *  |:: Lectura    ::|
 *  |>    3612     < |
 *  |:: Intensidad ::|
 *  |  | | | | |5    |
 *  |__maxima________|
 * 
 * */


char text[][6][20] = {
        {
            ":: Lectura    ::"
        },
        //  >  2612  <
        {
            ":: Intensidad ::"
        },
		{
			"      0", " |    1", " ||   2", " |||  3", " |||| 4", " |||||5"
		},
		{
			"_minima_", "_baja___", "_baja___", "_media__", "_alta___", "_maxima_"
		}
	};


static bool s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    //Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

static void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = EXAMPLE_READ_LEN,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000,
        .conv_mode = EXAMPLE_ADC_CONV_MODE,
        .format = EXAMPLE_ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = channel_num;
    for (int i = 0; i < channel_num; i++) {
        adc_pattern[i].atten = EXAMPLE_ADC_ATTEN;
        adc_pattern[i].channel = channel[i] & 0x7;
        adc_pattern[i].unit = EXAMPLE_ADC_UNIT;
        adc_pattern[i].bit_width = EXAMPLE_ADC_BIT_WIDTH;

        ESP_LOGI(TAG_ADC_READ_LOOP, "adc_pattern[%d].atten is :%"PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(TAG_ADC_READ_LOOP, "adc_pattern[%d].channel is :%"PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(TAG_ADC_READ_LOOP, "adc_pattern[%d].unit is :%"PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    *out_handle = handle;
}

// Modificacion de ssd1306_display_text_x3 a x2


void ssd1306_display_text_x2(SSD1306_t * dev, int page, const char * text, int text_len, bool invert)
{
	if (page >= dev->_pages) return;
	int _text_len = text_len;
	if (_text_len > 8) _text_len = 8;

	uint8_t seg = 0;

	for (uint8_t nn = 0; nn < _text_len; nn++) {

		uint8_t const * const in_columns = font8x8_basic_tr[(uint8_t)text[nn]];

		// make the character 2x as high
		out_column_t out_columns[8];
		memset(out_columns, 0, sizeof(out_columns));

		for (uint8_t xx = 0; xx < 8; xx++) { // for each column (x-direction)

			uint32_t in_bitmask = 0b1;
			uint32_t out_bitmask = 0b11;

			for (uint8_t yy = 0; yy < 8; yy++) { // for pixel (y-direction)
				if (in_columns[xx] & in_bitmask) {
					out_columns[xx].u32 |= out_bitmask;
				}
				in_bitmask <<= 1;
				out_bitmask <<= 2;
			}
		}

		// render character in 8 column high pieces, making them 2x as wide
		for (uint8_t yy = 0; yy < 2; yy++)	{ // for each group of 8 pixels high (y-direction)

			uint8_t image[16];
			for (uint8_t xx = 0; xx < 8; xx++) { // for each column (x-direction)
				image[xx*2+0] = 
				image[xx*2+1] = out_columns[xx].u8[yy];
			}
			if (invert) ssd1306_invert(image, 16);
			if (dev->_flip) ssd1306_flip(image, 16);
			if (dev->_address == SPIAddress) {
				spi_display_image(dev, page+yy, seg, image, 16);
			} else {
				i2c_display_image(dev, page+yy, seg, image, 16);
			}
			memcpy(&dev->_page[page+yy]._segs[seg], image, 16);
		}
		seg = seg + 16;
	}
}

// Layout personalizado oled

void disp_upd(SSD1306_t *dev, uint16_t data, const uint16_t max)
{
	if(data >= max)
		data = max - 1;

	char raw[10] = {'\0'};
	int _v_bar = 6 * data / max;

	sprintf(raw, "> %04hu <", data+1);

	ssd1306_display_text   (dev, 0, *text[0], 16, true);
	ssd1306_display_text_x2(dev, 1, raw, 8, false);
	ssd1306_display_text   (dev, 3, *text[1], 16, true);
	ssd1306_display_text_x2(dev, 4, text[2][_v_bar], 9, false);
	ssd1306_display_text_x2(dev, 6, text[3][_v_bar], 8, false);
}

void read_main(void)
{
	SSD1306_t dev[1] = {{._flip = false}};

	ESP_LOGI(tag, "INTERFACE is i2c");
	ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

	esp_err_t ret = ESP_OK;
    uint32_t ret_num = 0;
    uint8_t result[EXAMPLE_READ_LEN] = {0};
    memset(result, 0xcc, EXAMPLE_READ_LEN);

    s_task_handle = xTaskGetCurrentTaskHandle();

    adc_continuous_handle_t handle = NULL;
    continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &handle);

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(handle));

	
	ESP_LOGI(tag, "Panel is 128x64");
	
	// Inicializacion panel
	
	ssd1306_init(dev, 128, 64);
    ssd1306_clear_screen(dev, false);
	ssd1306_contrast(dev, 0xff);

	// Lectura
    
	uint32_t data = 0;
    uint32_t data_buff = UINT32_MAX;
    const uint32_t loop_period = 4;
    uint32_t current_loop_period = 0;
    
	disp_upd(dev, data, 4095);

    for(;; current_loop_period++) {
        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ret = adc_continuous_read(handle, result, EXAMPLE_READ_LEN, &ret_num, 0);
        if(ret == ESP_ERR_TIMEOUT ||  ret != ESP_OK || (current_loop_period < loop_period))
            continue;
        adc_digi_output_data_t *p = (adc_digi_output_data_t*)&result[DATA_READ_INIT];
        data = EXAMPLE_ADC_GET_DATA(p);
        current_loop_period = 0;

        if(fabs((double)(data - data_buff)) < 150)
            continue;

        data_buff = data;

        disp_upd(dev, data, 4095);
    }

    ESP_ERROR_CHECK(adc_continuous_stop(handle));
    ESP_ERROR_CHECK(adc_continuous_deinit(handle));

	esp_restart();
}
float get_average_single_read(void)
{
    // Realiza N lecturas consecutivas y obtiene el valor promedio

	esp_err_t ret = ESP_OK;
    uint32_t ret_num = 0;
    uint8_t result[EXAMPLE_READ_LEN] = {0};
    memset(result, 0xcc, EXAMPLE_READ_LEN);

    s_task_handle = xTaskGetCurrentTaskHandle();

    adc_continuous_handle_t handle = NULL;
    continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &handle);

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(handle));
	
	uint32_t i;
	float_t data = 0.f;
    
	for(i = 0; i < 4; i++) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ret = adc_continuous_read(handle, result, EXAMPLE_READ_LEN, &ret_num, 0);
        if(ret == ESP_ERR_TIMEOUT ||  ret != ESP_OK)
            continue;
        adc_digi_output_data_t *p = (adc_digi_output_data_t*)&result[DATA_READ_INIT];
        data = data + EXAMPLE_ADC_GET_DATA(p);
    }

    ESP_ERROR_CHECK(adc_continuous_stop(handle));
    ESP_ERROR_CHECK(adc_continuous_deinit(handle));

	return (float_t)data / i;
}