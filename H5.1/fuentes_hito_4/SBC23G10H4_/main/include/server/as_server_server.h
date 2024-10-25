#ifndef __AS_SERVER_SERVER__
#define __AS_SERVER_SERVER__

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"

#include "esp_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Handler to redirect incoming GET request for /index.html to /
 * This can be overridden by uploading file with same name 

static esp_err_t index_html_get_handler(httpd_req_t *req);*/


/* Handler to respond with an icon file embedded in flash.
 * Browsers expect to GET website icon at URI /favicon.ico.
 * This can be overridden by uploading file with same name

static esp_err_t favicon_get_handler(httpd_req_t *req);*/


/* Send HTTP response with a run-time generated html consisting of
 * a list of all files and folders under the requested path.
 * In case of SPIFFS this returns empty list when path is any
 * string other than '/', since SPIFFS doesn't support directories 

static esp_err_t http_resp_dir_html(httpd_req_t *req, const char *dirpath);*/

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

/* Set HTTP response content type according to file extension 

static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename);*/

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) 

static const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize);*/

/* Handler to download a file kept on the server 

static esp_err_t download_get_handler(httpd_req_t *req);*/

/* Handler to upload a file onto the server

static esp_err_t upload_post_handler(httpd_req_t *req);*/

/* Handler to delete a file from the server

static esp_err_t delete_post_handler(httpd_req_t *req);*/

/* Function to start the file server */

esp_err_t start_file_server(const char *base_path);


#ifdef __cplusplus
}
#endif


#endif /* !__AS_SERVER_SERVER__ */
