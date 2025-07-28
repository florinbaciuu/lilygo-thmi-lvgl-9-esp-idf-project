#pragma once
#ifndef COMMAND_LINE_INTERFACE_H_
#define COMMAND_LINE_INTERFACE_H_

#include "argtable3/argtable3.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "linenoise/linenoise.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "init.h"

#define MY_ESP_CONSOLE_CONFIG_DEFAULT()                                                            \
    {.max_cmdline_length = 256,                                                                    \
        .max_cmdline_args = 32,                                                                    \
        .heap_alloc_caps = MALLOC_CAP_DEFAULT,                                                     \
        .hint_color = 39,                                                                          \
        .hint_bold = 0}

// ------------------------------------

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

// register all commands
void cli_register_all_commands(void);

void StartCLI();

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */
#endif /* #ifndef COMMAND_LINE_INTERFACE_H_ */