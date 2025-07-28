

# ------------------------------ #
# RESTART MODULE
set(restart_srcs            "modules/restart/restart.c"
                            )
set(restart_includes        "modules/restart"
                            )
# ------------------------------ #
# ------------------------------ #
# TASK INFO MODULE
set(tasks_srcs              "modules/tasks_nfo/tasks_nfo.c"
                            )
set(tasks_includes          "modules/tasks_nfo"
                            )   
# ------------------------------ #
# ------------------------------ #
# UPTIME MODULE
set(uptime_srcs             "modules/uptime/uptime.c"
                            )
set(uptime_includes         "modules/uptime"
                            )
# ------------------------------ #
# ------------------------------ #
# INFO MODULE
set(info_srcs               "modules/info/info.c" 
                            "modules/info/funct.c"
                            )
set(info_includes           "modules/info"
                            )
# ------------------------------ #
# ------------------------------ #
# NVS MODULE
set(nvs_cmd_srcs            "modules/nvs_cmd/nvs_cmd.c"
                            )
set(nvs_cmd_includes        "modules/nvs_cmd"
                            )
# ------------------------------ #  
# ------------------------------ #
# WIFI MODULE
set(wifi_cmd_srcs           "modules/wifi_cmd/wifi_join_cmd.c"
                            )
set(wifi_cmd_includes       "modules/wifi_cmd"
                            )
# ------------------------------ #
# ------------------------------ #
# SET MODULE
set(set_cmd_srcs            "modules/set_cmd/set_cmd.c" 
                            "modules/set_cmd/set_log.c"
                            )
set(set_cmd_includes        "modules/set_cmd"
                            )
# ------------------------------ #


set(modules_srcs            ${restart_srcs} 
                            ${tasks_srcs}
                            ${uptime_srcs}
                            ${info_srcs}
                            ${nvs_cmd_srcs}
                            ${wifi_cmd_srcs}
                            ${set_cmd_srcs}
                            )

set(modules_includes        ${restart_includes} 
                            ${tasks_includes}
                            ${uptime_includes}
                            ${info_includes}
                            ${nvs_cmd_includes}
                            ${wifi_cmd_includes}
                            ${set_cmd_includes}
                            )

set(modules_priv_includes   ${restart_includes}
                            ${tasks_includes}
                            ${uptime_includes}
                            ${info_includes}
                            ${nvs_cmd_includes}
                            ${wifi_cmd_includes}
                            ${set_cmd_includes}
                            )

# ------------------------------- #

idf_component_register(
    SRCS                    "src/command_line_interface.c" 
                            "src/init.c" 
                            "src/config.c" 
                            ${modules_srcs}

    INCLUDE_DIRS            "." 
                            "include" 
                            ${modules_includes}

    PRIV_INCLUDE_DIRS       ${modules_priv_includes} 
                            

    REQUIRES                driver
                            freertos
                            esp_timer
                            console 
                            esp_hw_support 
                            spi_flash 
                            esp_system 
                            heap 
                            esp_common 
                            esp_psram 
                            esp_hw_support 
                            nvs_flash 
                            esp_wifi

    PRIV_REQUIRES           esp_bootloader_format
                            )

# Asta adaug-o separat:
set(KCONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Kconfig")

