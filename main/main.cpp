/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include "vcp.h"
#include "server.h"
#include "esp_log.h"

namespace {

const char *TAG = "main";

size_t on_bytes_received(char out[TX_BUFSZ], char* buf, size_t len) {
  vcp_send((uint8_t*)buf, len);
  return snprintf(out, TX_BUFSZ, "OK\n");
}

void handle_usb_rx(uint8_t *data, size_t data_len, void *arg) {
  ESP_LOGI(TAG, "USB recv: %.*s", data_len, data);
}

// TODO handle USB response

/**
 * @brief Main application
 *
 * This function shows how you can use VCP drivers
 */
extern "C" void app_main(void) {
  ESP_LOGI(TAG, "Setting up VCP");
  setup_vcp(&handle_usb_rx);
	run_server(&on_bytes_received);

  while (1) {
    loop_vcp();
  }
}

}
