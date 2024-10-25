#include <esp_netif.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <esp_random.h>
#include <stdio.h>

// Whether the given script is using encryption or not,
// generally recommended as it increases security (communication with the server is not in clear text anymore),
// it does come with an overhead tough as having an encrypted session requires a lot of memory,
// which might not be avaialable on lower end devices.

#include "thingsboard/Espressif_MQTT_Client.h"
#include "thingsboard/ThingsBoard.h"

#include "adc_read/adc_read.h"
#include "server/as_client_server.h"
#include "server/as_server_server.h"
#include "storage/storage.h"
#include "wifi/wifi_def.h"


// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
constexpr char TOKEN[] = "FiY8zgWzu1laHcYcnEkN";

// Thingsboard we want to establish a connection too
constexpr char THINGSBOARD_SERVER[] = "192.168.50.200";

// MQTT port used to communicate with the server, 1883 is the default unencrypted MQTT port,
// whereas 8883 would be the default encrypted SSL MQTT port
constexpr uint16_t THINGSBOARD_PORT = 1883U;

// Maximum size packets will ever be sent or received by the underlying MQTT client,
// if the size is to small messages might not be sent or received messages will be discarded
constexpr uint16_t MAX_MESSAGE_SIZE = 128U;

constexpr char LUMINOSITY_KEY[] = "luminosidad";


// Initalize the Mqtt client instance
struct Espressif_MQTT_Client mqttClient;
// Initialize ThingsBoard instance with the maximum needed buffer size
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

static const char *TAG = "SBC23G10H4_main";
static const char *WIFI_ENTRY_FORMAT = "%s , %s\n";
static const char *WIFI_CFG_FILE_NAME = "wifi.csv";
static const char *WIFI_CFG_FILE_PATH = "/data/wifi.csv";

static const char *WORKING_PATH = "/data";

static char wifi_ok = 0;
static float v = 0;

extern "C"
void app_main() {
    FILE *etc;
    esp_err_t ret;
    struct stat st;

    char w_ssid[35] = {'\0'};
    char w_pass[65] = {'\0'};
    int fi_parse;
    unsigned char fi_name[128] = {'\0'};
    unsigned char fi_content[1024] = {'\0'};

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize file storage
    ESP_ERROR_CHECK(mount_spiffs_storage(WORKING_PATH));

    //unlink("/data/wifi.csv");
    
    set_wifi_ok_addr(&wifi_ok);

    if (stat(WIFI_CFG_FILE_PATH, &st) == -1) {
        ESP_LOGI(TAG, "NO WIFI CFG!");
    } else if (!S_ISDIR(st.st_mode)) {
        etc = fopen(WIFI_CFG_FILE_PATH, "r");
        fi_parse = fscanf(etc, WIFI_ENTRY_FORMAT, w_ssid, w_pass);
        fclose(etc);
        
        if(fi_parse == 2)
            wifi_init_sta(w_ssid, w_pass);
        else
            ESP_LOGI(TAG, "WRONG WIFI CFG format !!!");
    }

    if(wifi_ok) {
        set_value_addr(&v);
        init_client_web_server();

        for(;;usleep(1000000)) {
            v = get_average_single_read();
            if (!tb.connected())
                tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT);
            tb.sendTelemetryData(LUMINOSITY_KEY, v);
            tb.loop();
        }
        
    } else {
        wifi_init_softap();
        start_file_server(WORKING_PATH);
    }
}