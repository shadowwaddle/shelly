#pragma once
#include <cstdint> 
#include <cstddef>
#include "esp_err.h"
#include "lwip/sockets.h"

class UdpLink {
public:
    // Create the socket, bind our listen port, remember where the laptop is.
    esp_err_t begin(const char* laptopIp, uint16_t laptopPort, uint16_t listenPort);

    // Send len bytes to the laptop. Returns false if sendto failed.
    bool send(const void* data, size_t len);

    // Wait up to timeoutMs for a packet. Returns bytes received, or -1 on timeout/error.
    int receive(void* buf, size_t bufSize, uint32_t timeoutMs);

private:
    int sock_ = -1;
    sockaddr_in laptop_ = {};   // filled once in begin(), reused by every send()
};