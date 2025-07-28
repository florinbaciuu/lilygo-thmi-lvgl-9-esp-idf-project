cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "pl_usb_device.cpp" "pl_usb_device_cdc.cpp" INCLUDE_DIRS "include" REQUIRES "esp_tinyusb" "pl_common")