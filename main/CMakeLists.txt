

cmake_minimum_required(VERSION 3.16)
# --------------------------------------------------- #
set(app_sources         "app_main.cpp"
                        "temp_sensor_cpu.cpp"
                        "init.cpp")
# --------------------------------------------------- #
set(app_include_dirs    "."
                        "")
# --------------------------------------------------- #
set(app_requires        driver
                        esp_bootloader_format
                        esp_psram
                        fatfs
                        spi_flash
                        esp_driver_usb_serial_jtag
                        esp_system
                        heap
                        console
                        esp_common
                        esp_hw_support
                        nvs_flash
                        esp_lcd
                        esp_wifi
                        esp-file-iterator
                        fmt
                        esp_lcd_touch
                        esp_lcd_touch_xpt2046
                        lvgl
                        esp_lvgl_port
                        )
# --------------------------------------------------- #
set(app_priv_requires   ${app_requires})
# --------------------------------------------------- #
# Set my components
# These are components that are not part of the IDF 
# but are used in this project.
# Put this components in /lib after set 
# set(EXTRA_COMPONENT_DIRS lib) in root/CMakeLists.txt
set(my_components_add   
onebutton-v0001
filesystem-v0002
)
# --------------------------------------------------- #
idf_component_register(
    SRCS                ${app_sources}
    INCLUDE_DIRS        ${app_include_dirs}
    REQUIRES            ${app_requires} 
                        ${my_components_add}
    PRIV_REQUIRES       ${app_priv_requires}
    WHOLE_ARCHIVE       True
    )

# --------------------------------------------------- #
