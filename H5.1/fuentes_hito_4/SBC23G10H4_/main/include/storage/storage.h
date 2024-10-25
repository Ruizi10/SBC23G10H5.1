#ifndef __STORAGE__
#define __STORAGE__

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"

#if SOC_SDMMC_HOST_SUPPORTED
#include "driver/sdmmc_host.h"
#endif
#include "sdmmc_cmd.h"
//#include "file_serving_example_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef CONFIG_EXAMPLE_MOUNT_SD_CARD

esp_err_t mount_sdcard_storage(const char* base_path);

#else

esp_err_t mount_spiffs_storage(const char* base_path);

#endif

esp_err_t start_file_server(const char *base_path);

//void check_credentials();

#ifdef __cplusplus
}
#endif

#endif /* !__STORAGE__ */