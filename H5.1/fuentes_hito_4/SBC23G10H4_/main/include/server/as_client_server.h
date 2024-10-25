#ifndef __AS_CLIENT_SERVER__
#define __AS_CLIENT_SERVER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <esp_http_server.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>

#ifdef __cplusplus
extern "C"
{
#endif

esp_err_t send_web_page(httpd_req_t *req);
esp_err_t get_req_handler(httpd_req_t *req);
httpd_handle_t init_client_web_server(void);
void set_value_addr(float *addr);

#ifdef __cplusplus
}
#endif
#endif /* !__AS_CLIENT_SERVER__ */
