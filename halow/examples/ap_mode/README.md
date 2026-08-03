<!--
Copyright 2026 Morse Micro

SPDX-License-Identifier: Apache-2.0
-->

# ap_mode

Starts a Wi-Fi HaLow interface in AP mode on a configurable S1G channel. Uses SAE security when a
passphrase is set. PMF is required and a single station is allowed.

---

## Building
Set the target, layering in the board profile for your board/hat, then configure and
build. See [configs](../../configs/README.md) for the full `SDKCONFIG_DEFAULTS` usage.

Setting the Wi-Fi HaLow country code is required — in `menuconfig` it is under
`Wi-Fi HaLow Connection Manager → Country code to use for Wi-Fi HaLow`
(`CONFIG_HALOW_COUNTRY_CODE`).

For example, for a Seeed Studio XIAO ESP32-S3 paired with a XIAO Wi-Fi HaLow
(MM6108) hat:

    SDKCONFIG_DEFAULTS="sdkconfig.defaults;managed_components/morsemicro__halow/configs/sdkconfig.defaults.seeed_xiao_esp32s3-seeed_xiao_mm6108" \
        idf.py set-target esp32s3
    idf.py menuconfig build   # set the country code (required)

## Running
Flash and open the serial monitor to watch the AP start and stations connect:

    idf.py flash monitor

## Configuration
Options under `HaLow ap_mode Example Configuration` (`idf.py menuconfig`):

| Option | Type | Default | Description |
| --- | --- | --- | --- |
| `CONFIG_WIFI_SSID` | string | `"MorseMicroESP32AP"` | SSID to advertise. |
| `CONFIG_WIFI_PSK` | string | `"12345678"` | Passphrase; leave empty for an open AP. |
| `CONFIG_S1G_CHANNEL` | int |  | S1G channel number to operate on. |
| `CONFIG_S1G_OPCLASS` | int |  | S1G operating class. |

The channel and operating class must be valid for the configured country code.
