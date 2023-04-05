/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>

#include "vcp.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "usb/cdc_acm_host.h"
#include "usb/vcp_ch34x.hpp"
#include "usb/vcp_cp210x.hpp"
#include "usb/vcp_ftdi.hpp"
#include "usb/vcp.hpp"
#include "usb/usb_host.h"

using namespace esp_usb;
using std::size_t;

// Change these values to match your needs
#define EXAMPLE_BAUDRATE     (115200)
#define EXAMPLE_STOP_BITS    (0)      // 0: 1 stopbit, 1: 1.5 stopbits, 2: 2 stopbits
#define EXAMPLE_PARITY       (0)      // 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space
#define EXAMPLE_DATA_BITS    (8)

namespace {
const char *TAG = "VCP example";
SemaphoreHandle_t device_disconnected_sem;

/**
 * @brief Data received callback
 *
 * Just pass received data to stdout
 */
void (*handle_rx)(uint8_t *data, size_t data_len, void *arg);

/**
 * @brief Device event callback
 *
 * Apart from handling device disconnection it doesn't do anything useful
 */
void handle_event(const cdc_acm_host_dev_event_data_t *event, void *user_ctx)
{
    switch (event->type) {
        case CDC_ACM_HOST_ERROR:
            ESP_LOGE(TAG, "CDC-ACM error has occurred, err_no = %d", event->data.error);
            break;
        case CDC_ACM_HOST_DEVICE_DISCONNECTED:
            ESP_LOGI(TAG, "Device suddenly disconnected");
            xSemaphoreGive(device_disconnected_sem);
            break;
        case CDC_ACM_HOST_SERIAL_STATE:
            ESP_LOGI(TAG, "Serial state notif 0x%04X", event->data.serial_state.val);
            break;
        case CDC_ACM_HOST_NETWORK_CONNECTION:
        default: break;
    }
}

/**
 * @brief USB Host library handling task
 *
 * @param arg Unused
 */
void usb_lib_task(void *arg)
{
    while (1) {
        // Start handling system events
        uint32_t event_flags;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            ESP_ERROR_CHECK(usb_host_device_free_all());
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            ESP_LOGI(TAG, "USB: All devices freed");
            // Continue handling USB events to allow device reconnection
        }
    }
}
}

void setup_vcp(void (*cb)(uint8_t*, size_t, void*))
{
    handle_rx = cb;

    device_disconnected_sem = xSemaphoreCreateBinary();
    assert(device_disconnected_sem);

    //Install USB Host driver. Should only be called once in entire application
    ESP_LOGI(TAG, "Installing USB Host");
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));

    // Create a task that will handle USB library events
    BaseType_t task_created = xTaskCreate(usb_lib_task, "usb_lib", 4096, NULL, 10, NULL);
    assert(task_created == pdTRUE);

    ESP_LOGI(TAG, "Installing CDC-ACM driver");
    ESP_ERROR_CHECK(cdc_acm_host_install(NULL));

    // Register VCP drivers to VCP service.
    VCP::register_driver<FT23x>();
    VCP::register_driver<CP210x>();
    VCP::register_driver<CH34x>();
}

static std::unique_ptr<CdcAcmDevice> vcp;
void vcp_send(uint8_t* data, size_t len) {
  // Send some dummy data
  ESP_LOGI(TAG, "USB tx: %.*s", len, data);
  ESP_ERROR_CHECK(vcp->tx_blocking(data, len));
  ESP_ERROR_CHECK(vcp->set_control_line_state(true, true));
}

void loop_vcp() {
  const cdc_acm_host_device_config_t dev_config = {
      .connection_timeout_ms = 5000, // 5 seconds, enough time to plug the device in or experiment with timeout
      .out_buffer_size = 64,
      .event_cb = handle_event,
      .data_cb = handle_rx,
      .user_arg = NULL,
  };

  // You don't need to know the device's VID and PID. Just plug in any device and the VCP service will pick correct (already registered) driver for the device
  ESP_LOGI(TAG, "Opening any VCP device...");
  vcp = std::unique_ptr<CdcAcmDevice>(VCP::open(&dev_config));

  if (vcp == nullptr) {
      ESP_LOGI(TAG, "Failed to open VCP device");
      return;
  }
  vTaskDelay(10);

  ESP_LOGI(TAG, "Setting up line coding");
  cdc_acm_line_coding_t line_coding = {
      .dwDTERate = EXAMPLE_BAUDRATE,
      .bCharFormat = EXAMPLE_STOP_BITS,
      .bParityType = EXAMPLE_PARITY,
      .bDataBits = EXAMPLE_DATA_BITS,
  };
  ESP_ERROR_CHECK(vcp->line_coding_set(&line_coding));

  xSemaphoreTake(device_disconnected_sem, portMAX_DELAY);
  ESP_LOGI(TAG, "VCP device disconnected");
}
