// Bench test, wifi connection betweeen laptop and esp32

#include "esp_err.h"
#include "lwip/sockets.h"
#include "net/UdpLink.hpp"
#include "net/Wifi.hpp"
#include "secrets.hpp"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include <cstdio>
#include <cstring>

static const char* TAG = "test_wifi";


static UdpLink udp;
static Wifi wifi;

extern "C" void app_main() {
    ESP_ERROR_CHECK(wifi.begin(secrets::kWifiSsid, secrets::kWifiPassword));
    ESP_ERROR_CHECK(udp.begin(secrets::kLaptopIp, 5000, 5001));

    int count = 0;
    while(true) {
        char buf[32];
        char rec[64];
        int len = snprintf(buf, sizeof(buf), "Hello %d", count);
        udp.send(buf, len);

        int n = udp.receive(rec, sizeof(rec), 1000);
        if (n > 0) {
            ESP_LOGI(TAG, "reply: %.*s", n, rec);
        }
        count++;
    }
}
