/*
 * Copyright 2026 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include "esp_netif.h"
#include "mmhalow.h"
#include "mmutils.h"
#include "mmwlan.h"
#include "nvs_flash.h"

static const char *TAG = "scan";

static SemaphoreHandle_t scan_done_sem = NULL;

static void scan_cb(enum mmwlan_scan_state state, void *args)
{
    ESP_LOGI(TAG, "Scan finished");
    xSemaphoreGive(scan_done_sem);
}

static int akm_str_len(uint32_t akm_suite)
{
    switch (akm_suite) {
        case MM_AKM_SUITE_NONE:
            return 4;
        case MM_AKM_SUITE_OTHER:
            return 5;
        default:
            return 3;
    }
}

static void rx_cb(const struct mmwlan_scan_result *entry, void *args)
{
    struct mm_rsn_information rsn = {0};
    mm_parse_rsn_information(entry->ies, entry->ies_len, &rsn);

    ESP_LOGI(TAG, "\n"
        "%.*s" "\n"
        "\t" "Operating BW: %d MHz" "\n"
        "\t" "BSSID: %02x:%02x:%02x:%02x:%02x:%02x" "\n"
        "\t" "RSSI: %3d" "\n"
        "\t" "Beacon Interval (TUs): %u" "\n"
        "\t" "Capability Info: 0x%04x" "\n"
        "\t" "Security: %.*s %.*s" "\n"
        ,
        entry->ssid_len,
        entry->ssid,
        entry->op_bw_mhz,
        entry->bssid[0],
        entry->bssid[1],
        entry->bssid[2],
        entry->bssid[3],
        entry->bssid[4],
        entry->bssid[5],
        entry->rssi,
        entry->beacon_interval,
        entry->capability_info,
        rsn.num_akm_suites ? akm_str_len(rsn.akm_suites[0]) : 4,
        rsn.num_akm_suites ? mm_akm_suite_to_string(rsn.akm_suites[0]) : "None",
        rsn.num_akm_suites > 1 ? akm_str_len(rsn.akm_suites[1]) : 0,
        rsn.num_akm_suites > 1 ? mm_akm_suite_to_string(rsn.akm_suites[1]) : ""
    );
}

void app_main(void)
{
    ESP_LOGI(TAG, "Booted. Initializing Wi-Fi HaLow...");

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());

    mmhalow_init(NULL);

    ESP_LOGI(TAG, "Wi-Fi HaLow Initialised");

    mmhalow_print_version_info();

    scan_done_sem = xSemaphoreCreateBinary();

    struct mmhalow_scan_args scargs = {
        .complete_cb = scan_cb,
        .rx_cb = rx_cb,
    };
    mmhalow_scan(&scargs);

    xSemaphoreTake(scan_done_sem, portMAX_DELAY);
    mmhalow_deinit();
}
