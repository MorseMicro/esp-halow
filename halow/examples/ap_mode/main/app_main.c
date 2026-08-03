/*
 * Copyright 2026 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include "esp_netif.h"
#include "mmhalow.h"
#include <unistd.h>
#include "nvs_flash.h"

#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PSK  CONFIG_WIFI_PSK
#define S1G_CHANNEL CONFIG_S1G_CHANNEL
#define S1G_OPCLASS CONFIG_S1G_OPCLASS
#define MAX_STA_CONN  1

static const char *TAG = "ap_mode";

void app_main(void)
{
    ESP_LOGI(TAG, "Booted. Initialising Wi-Fi HaLow Interface...");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_netif_init());
    mmhalow_init(NULL);
    ESP_LOGI(TAG, "Wi-Fi HaLow Interface Initialised");
    mmhalow_print_version_info();

    mmhalow_wifi_config_t wifi_config = { .ap = MMWLAN_AP_ARGS_INIT };

    memcpy((char *)wifi_config.ap.ssid, WIFI_SSID, strlen(WIFI_SSID));
    wifi_config.ap.ssid_len = strlen(WIFI_SSID);

    if (strlen(WIFI_PSK) > 0)
    {
        memcpy((char *)wifi_config.ap.passphrase, WIFI_PSK, strlen(WIFI_PSK));
        wifi_config.ap.security_type = MMWLAN_SAE;
    }
    else
    {
        wifi_config.ap.security_type = MMWLAN_OPEN;
    }

    wifi_config.ap.s1g_chan_num = S1G_CHANNEL;
    wifi_config.ap.max_stas = MAX_STA_CONN;
    wifi_config.ap.pmf_mode = MMWLAN_PMF_REQUIRED;
    wifi_config.ap.op_class = S1G_OPCLASS;

    mmhalow_set_config(WIFI_IF_AP, &wifi_config);

    ESP_LOGI(TAG, "Attemping to start AP");
    mmhalow_wifi_start();
    ESP_LOGI(TAG, "AP Started");
}
