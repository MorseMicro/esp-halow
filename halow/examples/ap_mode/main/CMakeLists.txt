# Copyright 2026 Morse Micro
#
# SPDX-License-Identifier: Apache-2.0

idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES halow nvs_flash esp_wifi
)
