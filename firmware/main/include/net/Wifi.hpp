#pragma once
#include <cstdint>
#include "esp_err.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// Joins a Wi-Fi network as a station (a client, like a phone does)
// and keeps reconnecting forever if the connection drops.
class Wifi {
public:
    // Starts Wi-Fi and waits until we have an IP address, or timeoutMs passes.
    // On timeout it returns ESP_ERR_TIMEOUT but keeps trying in the background.
    esp_err_t begin(const char* ssid, const char* password, uint32_t timeoutMs = 15000);

    // True while we are connected and have an IP address.
    bool isConnected() const;

private:
    // ESP-IDF calls this (from its event task) whenever Wi-Fi or IP has news.
    // It must be static: C callbacks can't take a hidden `this` pointer,
    // so we pass `this` in through the `arg` parameter instead.
    static void onEvent(void* arg, esp_event_base_t base, int32_t id, void* data);

    EventGroupHandle_t events_ = nullptr;
};
