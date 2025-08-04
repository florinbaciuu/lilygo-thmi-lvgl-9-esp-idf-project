
# Se adauga modulul hello
set(hello_cmd_srcs "modules/hello/hello.c")
set(hello_cmd_includes "modules/hello")

# Se adauga restart
set(restart_srcs "modules/restart/restart.c")
set(restart_includes "modules/restart")

# Se adauga task info
set(tasks_srcs "modules/tasks_nfo/tasks_nfo.c")
set(tasks_includes "modules/tasks_nfo")


# Se adauga uptime
set(uptime_srcs "modules/uptime/uptime.c")
set(uptime_includes "modules/uptime")

# Se adauga modulul info
set(info_srcs "modules/info/info.c" "modules/info/funct.c" "modules/info/stack.c")
set(info_includes "modules/info")

# Se adauga modulul nvs
set(nvs_cmd_srcs "modules/nvs_cmd/nvs_cmd.c")
set(nvs_cmd_includes "modules/nvs_cmd")

# Se adauga modulul wifi
set(wifi_cmd_srcs "modules/wifi_cmd/wifi_join_cmd.c")
set(wifi_cmd_includes "modules/wifi_cmd")

# Se adauga modulul set
set(set_cmd_srcs "modules/set_cmd/set_cmd.c" "modules/set_cmd/set_log.c")
set(set_cmd_includes "modules/set_cmd")

# ------------------------------ #

# Aduna toate modulele intr-un singur
set(modules_srcs ${hello_cmd_srcs} ${restart_srcs} ${tasks_srcs}  ${uptime_srcs} ${info_srcs} ${nvs_cmd_srcs} ${wifi_cmd_srcs} ${set_cmd_srcs})
set(modules_includes ${hello_cmd_includes} ${restart_includes} ${tasks_includes}  ${uptime_includes} ${info_includes} ${nvs_cmd_includes} ${wifi_cmd_includes} ${set_cmd_includes})
set(modules_priv_includes ${hello_cmd_includes} ${restart_includes} ${tasks_includes}  ${uptime_includes} ${info_includes} ${nvs_cmd_includes} ${wifi_cmd_includes} ${set_cmd_includes})

# ------------------------------- #

idf_component_register(
    SRCS "src/command_line_interface.c" "src/init.c" "src/config.c" ${modules_srcs}
    INCLUDE_DIRS "." "include" ${modules_includes}
    PRIV_INCLUDE_DIRS ${modules_priv_includes} ${REQUIRES}
    REQUIRES driver freertos esp_timer console esp_hw_support spi_flash esp_system heap esp_common esp_psram esp_hw_support nvs_flash esp_wifi
)

# Asta adaug-o separat:
set(KCONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Kconfig")

# Activează automat definirea opțiunilor Kconfig
#target_compile_definitions(${COMPONENT_LIB} PRIVATE "-DRESOURCE_MONITOR_CONFIG_FROM_KCONFIG")

