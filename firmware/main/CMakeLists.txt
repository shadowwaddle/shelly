file(GLOB_RECURSE SRCS "src/*.cpp")

# Pick ONE entry point (only one app_main per build):
#   idf.py -DBENCH_TEST=imu   build   -> test/test_imu.cpp
#   idf.py -DBENCH_TEST=OFF   build   -> main.cpp (the real robot)
if(BENCH_TEST)
    set(ENTRY "test/test_${BENCH_TEST}.cpp")
else()
    set(ENTRY "main.cpp")
endif()

idf_component_register(
    SRCS ${ENTRY} ${SRCS}
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_gpio esp_driver_ledc esp_driver_i2c esp_timer esp_rom esp32-camera
             esp_wifi esp_netif esp_event nvs_flash lwip
)