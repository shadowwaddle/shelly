#include "net/UdpLink.hpp"
#include "esp_log.h"
#include "lwip/arch.h"
#include "lwip/sockets.h"
#include <cerrno>
#include <sys/_timeval.h>

static const char* TAG = "UdpLink";

// Create the socket, bind our listen port, remember where the laptop is.
esp_err_t UdpLink::begin(const char* laptopIp, uint16_t laptopPort, uint16_t listenPort) {
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ < 0) {
        ESP_LOGE(TAG, "socket() failed: errno %d", errno);
        return ESP_FAIL;
    }
    
    sockaddr_in me = {};
    me.sin_addr.s_addr = htonl(INADDR_ANY);
    me.sin_port = htons(listenPort);
    me.sin_family = AF_INET;

    if(bind(sock_, (sockaddr*)&me, sizeof(me)) != 0) {
        ESP_LOGE(TAG, "bind() failed: errno %d", errno);
        close(sock_);
        sock_ = -1;
        return ESP_FAIL;
    }

    laptop_ = {};
    laptop_.sin_family = AF_INET;
    laptop_.sin_port = htons(laptopPort);

    if (inet_pton(AF_INET, laptopIp, &laptop_.sin_addr) != 1) {
        ESP_LOGE(TAG, "bad laptop IP: %s", laptopIp);
        close(sock_);
        sock_ = -1;
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "listening on %d, sending to %s:%d", listenPort, laptopIp, laptopPort);
    return ESP_OK;
}

// Send len bytes to the laptop. Returns false if sendto failed.
bool UdpLink::send(const void* data, size_t len) {
    if (sock_ < 0) {
        ESP_LOGE(TAG, "no socket or being has failed");
        return false;
    }
    if (sendto(sock_, data, len, 0, (sockaddr*)&laptop_, sizeof(laptop_)) < 0) {
        ESP_LOGE(TAG, "sendto() failed: errno %d", errno);
        return false;
    }
    return true;
}

// Wait up to timeoutMs for a packet. Returns bytes received, or -1 on timeout/error.
int UdpLink::receive(void* buf, size_t bufSize, uint32_t timeoutMs) {
     if (sock_ < 0) {
        ESP_LOGE(TAG, "no socket or receive() called before begin()");
        return -1;
    }
    timeval tv = {};
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    setsockopt(sock_, SOL_SOCKET , SO_RCVTIMEO, &tv, sizeof(tv));
    return recvfrom(sock_, buf, bufSize, 0, nullptr, nullptr);
}

