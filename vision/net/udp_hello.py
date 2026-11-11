# vision/net/udp_hello.py — laptop side of the Wi-Fi bench test
import socket

LISTEN_PORT = 5000     # must match laptopPort in udp.begin(...)
ESP_PORT    = 5001     # must match listenPort in udp.begin(...)

# Like UdpLink::begin(): create a socket and bind our port
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", LISTEN_PORT))
print(f"listening on port {LISTEN_PORT}...")

while True:
    # Like UdpLink::receive() (no timeout: waits until something arrives)
    data, addr = sock.recvfrom(1024)
    esp_ip = addr[0]
    print(f"from {esp_ip}: {data.decode()}")

    # Like UdpLink::send(): reply to the ESP32's port
    sock.sendto(b"got " + data, (esp_ip, ESP_PORT))