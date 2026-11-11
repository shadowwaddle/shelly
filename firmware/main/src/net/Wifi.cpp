// Adapted from ESP-IDF's examples/wifi/getting_started/station.
#include "net/Wifi.hpp"
#include <cinttypes>
#include <cstring>
#include "esp_check.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

namespace {
    constexpr const char* TAG = "Wifi";
    constexpr EventBits_t kConnectedBit = 1 << 0;   // set = we have an IP address

    // wifi_config_t stores the SSID/password as fixed-size byte arrays, not strings,
    // so we copy the text in by hand and refuse anything that won't fit.
    bool copyText(uint8_t* dst, size_t dstSize, const char* src) {
        size_t len = strlen(src);
        if (len > dstSize) return false;
        memcpy(dst, src, len);   // the struct was zeroed, so the rest stays 0
        return true;
    }
}

esp_err_t Wifi::begin(const char* ssid, const char* password, uint32_t timeoutMs) {
    // 1) NVS: flash storage the Wi-Fi driver keeps calibration data in.
    //    If it's full or from an older IDF version, wipe it and try again.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "nvs erase failed");
        err = nvs_flash_init();
    }
    ESP_RETURN_ON_ERROR(err, TAG, "nvs init failed");

    // 2) The flags the event handler uses to tell begin() "connected".
    events_ = xEventGroupCreate();
    if (events_ == nullptr) return ESP_ERR_NO_MEM;

    // 3) Networking software (lwIP), the event loop, and the
    //    interface that connects the Wi-Fi radio to lwIP.
    ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "netif init failed");
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {   // INVALID_STATE = already created: fine
        return err;
    }
    esp_netif_create_default_wifi_sta();

    // 4) Start the Wi-Fi driver (radio still off).
    wifi_init_config_t initCfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&initCfg), TAG, "wifi init failed");

    // 5) "Call onEvent about these events, and hand it this object."
    ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &Wifi::onEvent, this, nullptr), TAG, "register failed");
    ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &Wifi::onEvent, this, nullptr), TAG, "register failed");

    // 6) Which network to join.
    wifi_config_t wc = {};
    if (!copyText(wc.sta.ssid, sizeof(wc.sta.ssid), ssid) ||
        !copyText(wc.sta.password, sizeof(wc.sta.password), password)) {
        ESP_LOGE(TAG, "SSID or password too long");
        return ESP_ERR_INVALID_ARG;
    }
    // Weakest security we'll accept. WPA2 also allows WPA3 networks.
    wc.sta.threshold.authmode = (strlen(password) == 0) ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;
    wc.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "set mode failed");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wc), TAG, "set config failed");

    // 7) Radio on. The rest happens in onEvent: START -> connect -> GOT_IP.
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "wifi start failed");

    // Power saving lets the radio doze between router beacons, which can delay
    // incoming packets by 100+ ms. A car taking steering commands wants it off.
    esp_wifi_set_ps(WIFI_PS_NONE);

    // 8) Sleep until onEvent sets the connected bit, or we time out.
    EventBits_t bits = xEventGroupWaitBits(events_, kConnectedBit,
                                           pdFALSE,   // don't clear the bit when we wake
                                           pdTRUE,    // wait for all listed bits (just one here)
                                           pdMS_TO_TICKS(timeoutMs));
    if (!(bits & kConnectedBit)) {
        ESP_LOGW(TAG, "no IP after %" PRIu32 " ms (still trying in the background)", timeoutMs);
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

bool Wifi::isConnected() const {
    return events_ != nullptr && (xEventGroupGetBits(events_) & kConnectedBit);
}

void Wifi::onEvent(void* arg, esp_event_base_t base, int32_t id, void* data) {
    // `arg` is the `this` we passed when registering: turn it back into our object.
    auto* self = static_cast<Wifi*>(arg);

    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();   // radio is on: start joining

    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        // Lost the network, or couldn't join it. Unlike the example,
        // never give up: a moving car will drop out and come back.
        xEventGroupClearBits(self->events_, kConnectedBit);
        auto* info = static_cast<wifi_event_sta_disconnected_t*>(data);
        // Reason codes are listed in esp_wifi_types.h (wifi_err_reason_t).
        // A wrong password usually shows up as 15 or 204.
        ESP_LOGW(TAG, "disconnected (reason %d), retrying", info->reason);
        esp_wifi_connect();

    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        auto* info = static_cast<ip_event_got_ip_t*>(data);
        ESP_LOGI(TAG, "got IP " IPSTR, IP2STR(&info->ip_info.ip));
        xEventGroupSetBits(self->events_, kConnectedBit);
    }
}
