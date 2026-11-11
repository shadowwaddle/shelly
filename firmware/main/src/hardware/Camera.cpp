#include "hardware/Camera.hpp"
#include "config.hpp"
#include "esp_camera.h"
#include "esp_camera_af.h"
#include "esp_err.h"
#include "esp_log.h"


static const char* TAG = "Camera";

esp_err_t Camera::begin() {
    // Initialize all the pins
    camera_config_t cfg = {};
    cfg.pin_pwdn = -1;
    cfg.pin_reset = -1;
    cfg.pin_xclk = config::kCamXclk;
    cfg.pin_sccb_sda = config::kCamSda;
    cfg.pin_sccb_scl = config::kCamScl;
    cfg.pin_d0 = config::kCamD0;
    cfg.pin_d1 = config::kCamD1;
    cfg.pin_d2 = config::kCamD2;
    cfg.pin_d3 = config::kCamD3;
    cfg.pin_d4 = config::kCamD4;
    cfg.pin_d5 = config::kCamD5;
    cfg.pin_d6 = config::kCamD6;
    cfg.pin_d7 = config::kCamD7;
    cfg.pin_vsync = config::kCamVsync;
    cfg.pin_href = config::kCamHref;
    cfg.pin_pclk = config::kCamPclk;
    cfg.xclk_freq_hz = 20000000; // Clock speed

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed");
        return err;
    }

    return ESP_OK;
}
camera_fb_t* Camera::grab() {
    
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed!");
        return nullptr;
    }
    return fb;
}

void Camera::release(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}
