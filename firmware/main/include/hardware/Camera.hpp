#pragma once
#include "esp_camera.h"

class Camera {
    public:
        esp_err_t begin();
        camera_fb_t* grab();
        void release(camera_fb_t* fb);
};