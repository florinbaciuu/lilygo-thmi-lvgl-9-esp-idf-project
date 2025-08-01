/**
 * @file      app_main.cpp.
 * @author    Baciu Aurel Florin
 * @brief     Main application file for the Lilygo T HMI development board.
 * @brief     Demo
 * @license   MIT
 * @copyright Copyright (c) 2025 Baciu Aurel Florin
 * @copyright Oltean East
 * @version   1.0.0
 *
 * @date      10 July 2025
 * @time      20:40 (PM)
 * @computer  Mac Mini M4
 * ..
 * @details   This file contains the main application logic, including initialization of hardware
 * components,
 */

/*********************
 *      DEFINES
 *********************/
#define BOARD_TFT_DATA0 (48) // GPIO pin for TFT data line 0
#define BOARD_TFT_DATA1 (47) // GPIO pin for TFT data line 1
#define BOARD_TFT_DATA2 (39) // GPIO pin for TFT data line 2
#define BOARD_TFT_DATA3 (40) // GPIO pin for TFT data line 3
#define BOARD_TFT_DATA4 (41) // GPIO pin for TFT data line 4
#define BOARD_TFT_DATA5 (42) // GPIO pin for TFT data line 5
#define BOARD_TFT_DATA6 (45) // GPIO pin for TFT data line 6
#define BOARD_TFT_DATA7 (46) // GPIO pin for TFT data line 7
#define BOARD_TFT_RST (-1)   // GPIO pin for TFT reset, set to -1 if not used
#define BOARD_TFT_CS (6)     // GPIO pin for TFT chip select
#define BOARD_TFT_DC (7)     // GPIO pin for TFT data/command control
#define BOARD_TFT_WR (8)     // GPIO pin for TFT write control
#define LCD_WIDTH (320)      // Width of the LCD in pixels
#define LCD_HEIGHT (240)     // Height of the LCD in pixels
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (10 * 1000 * 1000) // LCD pixel clock frequency in Hz
#define BOARD_TFT_BL (38)                             // GPIO pin for backlight control
#define PWR_EN_PIN (10)                               // connected to the battery alone
//---------
#define PWR_ON_PIN                                                                                 \
    (14) // if you use an ext 5V power supply, you need to bring a magnet close to the ReedSwitch
         // and set the PowerOn Pin (GPIO14) to HIGH
#define Dellp_OFF_PIN                                                                              \
    (21) // connected to the battery and the USB power supply, used to turn off the device
//---------
#define PIN_NUM_IRQ (9)  // IRQ pin for touch controller
#define BAT_ADC_PIN (5)  // ADC pin for battery voltage measurement
#define PIN_NUM_MISO (4) // MISO pin for touch controller
#define PIN_NUM_MOSI (3) // MOSI pin for touch controller
#define PIN_NUM_CLK (1)  // CLK pin for touch controller
#define PIN_NUM_CS (2)   // CS pin for touch controller

/*********************
 *    LVGL DEFINES
 *********************/
/* LVGL TASK NOTIFICATION */
#define USE_MUTEX 0
#define USE_FREERTOS_TASK_NOTIF 1  // cica e mai rapid cu 20 %
#define LV_TASK_NOTIFY_SIGNAL 0x01 // Semnalul pentru notificarea LVGL
////#define LV_TASK_NOTIFY_SIGNAL_MODE USE_MUTEX
#define LV_TASK_NOTIFY_SIGNAL_MODE (USE_FREERTOS_TASK_NOTIF)
//---------
/* BUFFER MODE */
#define BUFFER_20LINES 0
#define BUFFER_40LINES 1
#define BUFFER_60LINES 2 // merge
#define BUFFER_DEVIDED4 3
#define BUFFER_FULL 4             // merge super ok
#define BUFFER_MODE (BUFFER_FULL) // selecteaza modul de buffer , defaut este BUFFER_FULL
#define DOUBLE_BUFFER_MODE (true)
//---------
/* BUFFER MEMORY TYPE AND DMA */
#define BUFFER_INTERNAL 0
#define BUFFER_SPIRAM 1
#define BUFFER_MEM (BUFFER_SPIRAM)
#if (BUFFER_MEM == BUFFER_INTERNAL)
#define DMA_ON (true)
#endif
//---------
/* RENDER MODE */
#define RENDER_MODE_PARTIAL 0 // Modul recomandat pt dual buffer and no canvas and no direct mode
#define RENDER_MODE_FULL 1    //
#define RENDER_MODE (RENDER_MODE_PARTIAL) // selecteaza modul de randare
//---------

/*********************
 *      INCLUDES
 *********************/
extern "C" {
#include "esp_bootloader_desc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define ESP_LCD_IO_I2C_SKIP_INCLUDE 1
#include "command_line_interface.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h" // Sau driverul real folosit de tine
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_xpt2046.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "ui.h"
#include <lv_conf.h>
}

#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/color.h>

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// ----------------------------------------------------------

/**
 * Prototypes for functions
 */

// ----------------------------------------------------------

// ----------------------------------------------------------

/**********************
 *   GLOBAL VARIABLES
 **********************/
RTC_DATA_ATTR static uint32_t boot_count = 0;
esp_lcd_panel_io_handle_t touch_io_handle = NULL;
esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
esp_lcd_i80_bus_handle_t i80_bus = NULL;
esp_lcd_touch_handle_t touch_handle = NULL;
esp_lcd_panel_handle_t panel_handle = NULL;
//---------

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
//---------
void gpio_extra_set_init(uint32_t mode) {
    // Setăm ambii pini ca output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PWR_EN_PIN) | (1ULL << PWR_ON_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)PWR_EN_PIN, mode);
    gpio_set_level((gpio_num_t)PWR_ON_PIN, mode); // nu e nevoie de el daca alimentam usb
}
//---------
void power_latch_init() {
    gpio_config_t io_conf = {.pin_bit_mask = 1ULL << PWR_EN_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)PWR_EN_PIN, 1); // ⚡ ține placa aprinsă
}
//---------
void gfx_set_backlight(uint32_t mode) {
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BOARD_TFT_BL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)BOARD_TFT_BL, mode);
}
//---------

/**********************
 *   TOUCH VARIABLES
 **********************/
int16_t touch_map_x1 = 3857;
int16_t touch_map_x2 = 239;
int16_t touch_map_y1 = 213;
int16_t touch_map_y2 = 3693;
uint16_t x = 0, y = 0;
uint8_t num_points = 0;

/**********************
 *   TOUCH FUNCTIONS
 **********************/
int touch_map_value(int val, int in_min, int in_max, int out_min, int out_max) {
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//---------
void touch_get_calibrated_point(int16_t xraw, int16_t yraw, int16_t* x_out, int16_t* y_out) {
    *x_out = touch_map_value(xraw, touch_map_x1, touch_map_x2, 0, LCD_WIDTH - 1);
    *y_out = touch_map_value(yraw, touch_map_y1, touch_map_y2, 0, LCD_HEIGHT - 1);
    if (*x_out < 0) {
        *x_out = 0;
    }
    if (*x_out >= LCD_WIDTH) {
        *x_out = LCD_WIDTH - 1;
    }
    if (*y_out < 0) {
        *y_out = 0;
    }
    if (*y_out >= LCD_HEIGHT) {
        *y_out = LCD_HEIGHT - 1;
    }
}
//---------
bool touch_read(uint16_t* x_out, uint16_t* y_out) {
    uint16_t x_raw = 0, y_raw = 0;
    uint8_t point_count = 0;
    esp_lcd_touch_read_data(touch_handle);
    bool touched =
        esp_lcd_touch_get_coordinates(touch_handle, &x_raw, &y_raw, NULL, &point_count, 1);
    if (touched && point_count > 0) {
        int16_t x_cal, y_cal;
        touch_get_calibrated_point(x_raw, y_raw, &x_cal, &y_cal);
        if (x_out) {
            *x_out = x_cal;
        }
        if (y_out) {
            *y_out = y_cal;
        }
        return true;
    }
    return false;
}
//---------
bool touch_panel_is_touched(void) {
    uint16_t x_raw = 0, y_raw = 0;
    uint8_t point_count = 0;
    esp_lcd_touch_read_data(touch_handle);
    bool is_touched =
        esp_lcd_touch_get_coordinates(touch_handle, &x_raw, &y_raw, NULL, &point_count, 1);
    return is_touched && point_count > 0;
}
//---------

/**********************
 *   LVGL VARIABLES
 **********************/
uint32_t bufSize;             // Dimensiunea buffer-ului
lv_color_t* disp_draw_buf;    // Buffer LVGL
lv_color_t* disp_draw_buf_II; // Buffer LVGL secundar
lv_display_t* disp;           // Display LVGL

/**********************
 *   LVGL FUNCTIONS
 **********************/
/* Display flushing function callback */
void lv_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    esp_lcd_panel_draw_bitmap(
        panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (const void*)px_map);
    lv_disp_flush_ready(disp);
}
//---------
void lv_touchpad_read(lv_indev_t* indev_drv, lv_indev_data_t* data) {
    static uint16_t last_x = 0;
    static uint16_t last_y = 0;
    uint16_t x, y;
    if (touch_read(&x, &y)) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;
        last_x = x;
        last_y = y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED; // Nu e apăsat → eliberat
        data->point.x = last_x; // Păstrăm ultima poziție x (LVGL o cere chiar și în RELEASED)
        data->point.y = last_y; // Păstrăm ultima poziție y (LVGL o cere chiar și în RELEASED)
    }
}
//---------
void lv_touchpad_read_v2(lv_indev_t* indev_drv, lv_indev_data_t* data) {
    static uint16_t last_x = 0;   // Ultima poziție X
    static uint16_t last_y = 0;   // Ultima poziție Y
    static uint16_t stable_x = 0; // Poziția stabilă X
    static uint16_t stable_y = 0; // Poziția stabilă Y
    static uint8_t touch_cnt = 0; // Numărul de atingeri stabile
    static const uint8_t touch_tolerance =
        8; // Poți crește sau micșora după feeling // Distanța permisă între citiri succesive
    static const uint8_t TOUCH_STABLE_THRESHOLD =
        0; // Threshold pentru stabilitate  // De câte ori trebuie să fie stabil ca să fie
           // considerat apăsat
    uint16_t x, y;
    if (touch_read(&x, &y)) {
        if (abs(x - last_x) < touch_tolerance && abs(y - last_y) < touch_tolerance) {
            if (touch_cnt < 255) {
                touch_cnt++; // Incrementăm numărul de atingeri stabile
            }
        } else {
            touch_cnt = 0; // Resetăm dacă mișcarea e prea mare
        }
        last_x = x;
        last_y = y;
        if (touch_cnt >= TOUCH_STABLE_THRESHOLD) {
            data->state = LV_INDEV_STATE_PRESSED;
            stable_x = x;
            stable_y = y;
        } else {
            data->state = LV_INDEV_STATE_RELEASED; // Nu trimitem touch activ până nu e stabil
        }
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    data->point.x = stable_x; // Trimitem ultima poziție stabilă
    data->point.y = stable_y; // Trimitem ultima poziție stabilă
}
//---------
bool panel_io_trans_done_callback(
    esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx) {
    if (disp != NULL) {
    } else {
        esp_rom_printf("[lv_trans_done_cb] - disp este NULL!\n");
    }
    return false; // false înseamnă: nu mai face nimic după
}
//---------
uint32_t lv_get_rtos_tick_count_callback(void) {
    return xTaskGetTickCount();
} // Callback pentru a obține numărul de tick-uri RTOS
//--------------------------------------

/*
███████ ██████  ███████ ███████ ██████ ████████  ██████  ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██ ██      
█████   ██████  █████   █████   ██████    ██    ██    ██ ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██      ██ 
██      ██   ██ ███████ ███████ ██   ██   ██     ██████  ███████ 
*/

/*********************
 *  rtos variables
 *********************/
TaskHandle_t xHandle_lv_main_task;
TaskHandle_t xHandle_lv_main_tick_task;
TaskHandle_t xHandle_chechButton0State;
//---------
#if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
SemaphoreHandle_t lvgl_mutex;
bool lv_port_sem_take(void) {
    return (xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE);
}
//---------
bool lv_port_sem_give(void) {
    return (xSemaphoreGive(lvgl_mutex) == pdTRUE);
}
#endif // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)

/********************************************** */
/*                   TASK                       */
/********************************************** */
#if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
void lv_main_tick_task(void* parameter) {
    static TickType_t tick = 0;
    tick = xTaskGetTickCount(); // Inițializare corectă
    while (true) {
        lv_tick_inc(5);                           // Incrementeaza tick-urile la fiecare 5ms
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(5)); // Delay precis mult mai rapid asa
    }
}
#elif (LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF)
void lv_main_tick_task(void* parameter) {
    static TickType_t tick = 0;
    tick = xTaskGetTickCount();
    while (true) {
        lv_tick_inc(5); // Incrementeaza tick-urile LVGL
        xTaskNotify(
            xHandle_lv_main_task, LV_TASK_NOTIFY_SIGNAL, eSetBits); // Notifica task-ul principal
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(5)); // Delay precis mult mai rapid asa
    }
}
#endif // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF)

/********************************************** */
/*                   TASK                       */
/********************************************** */
#if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
void lv_main_task(void* parameter) {
    static TickType_t tick = 0;
    tick = xTaskGetTickCount(); // Inițializare corectă
    while (true) {
        if (lv_port_sem_take()) // Protejeaza accesul la LVGL
        {
            lv_timer_handler(); /* let the GUI do its work */
            lv_port_sem_give(); // Eliberam mutex-ul
        }
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(5)); // Delay precis mult mai rapid asa
    }
}
#endif // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
#if LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF
void lv_main_task(void* parameter) {
    static TickType_t tick = 0;
    tick = xTaskGetTickCount();                         // Inițializare corectă
    xHandle_lv_main_task = xTaskGetCurrentTaskHandle(); // Încoronarea oficială
    while (true) {
        uint32_t notificationValue;                 // Așteapta notificarea
        BaseType_t notified = xTaskNotifyWait(0x00, // Niciun bit de ignorat
            ULONG_MAX,                              // Curata toate biturile
            &notificationValue,                     // Primește valoarea notificarii
            portMAX_DELAY);                         // Așteapta notificarea pe termen nelimitat
        if (notified == pdTRUE && (notificationValue & LV_TASK_NOTIFY_SIGNAL)) {
            lv_timer_handler(); // LVGL își face treaba
        }
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(5)); // Delay precis mult mai rapid asa
    }
}
#endif // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF)

/********************************************** */
/*                   TASK                       */
/********************************************** */
static void IRAM_ATTR chechButton0State_isr_handler(void* arg) {
    // NOTĂ: NU face log sau delay aici
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR((TaskHandle_t)arg, 0x01, eSetBits, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}
//---------
void chechButton0State(void* parameter) {
    (void)parameter;
    xHandle_chechButton0State = xTaskGetCurrentTaskHandle(); // Încoronarea oficială
    uint32_t notificationValue;
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << 0,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // activăm pull-up intern
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE, // întrerupere pe orice schimbare de stare
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1); // doar o dată în tot proiectul
    gpio_isr_handler_add(
        (gpio_num_t)0, chechButton0State_isr_handler, (void*)xHandle_chechButton0State);
    while (true) {
        xTaskNotifyWait(0x00, 0xFFFFFFFF, &notificationValue, portMAX_DELAY);
        if (notificationValue & 0x01) {
            ESP_LOGW("BUTTON", "Button ACTIVAT pe GPIO0");
            //// Acțiune custom (PUNE COD AICI)
        }
        vTaskDelay(200);
    }
}
/****************************/

//--------------------------------------

/*
███    ███  █████  ██ ███    ██ 
████  ████ ██   ██ ██ ████   ██ 
██ ████ ██ ███████ ██ ██ ██  ██ 
██  ██  ██ ██   ██ ██ ██  ██ ██ 
██      ██ ██   ██ ██ ██   ████ 
  * This is the main entry point of the application.
  * It initializes the hardware, sets up the display, and starts the LVGL tasks.
  * The application will run indefinitely until the device is powered off or reset.
*/
extern "C" void app_main(void) {
    power_latch_init(); // Inițializare latch pentru alimentare
    gfx_set_backlight(1);
    esp_log_level_set("*", ESP_LOG_INFO);

    boot_count++;
    ESP_LOGI("RTC", "Boot count (from RTC RAM): %lu", boot_count);
    esp_sleep_enable_timer_wakeup(5000000); // 5 secunde în microsecunde

    esp_bootloader_desc_t bootloader_desc;
    printf("\n");
    ESP_LOGI("Bootloader description", "\tESP-IDF version from 2nd stage bootloader: %s\n",
        bootloader_desc.idf_ver);
    ESP_LOGI("Bootloader description", "\tESP-IDF version from app: %s\n", IDF_VER);
    // printf("\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
    // printf("\tESP-IDF version from app: %s\n", IDF_VER);

    esp_lcd_i80_bus_config_t lcd_bus_config = {.dc_gpio_num = BOARD_TFT_DC,
        .wr_gpio_num = BOARD_TFT_WR,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .data_gpio_nums =
            {
                BOARD_TFT_DATA0,
                BOARD_TFT_DATA1,
                BOARD_TFT_DATA2,
                BOARD_TFT_DATA3,
                BOARD_TFT_DATA4,
                BOARD_TFT_DATA5,
                BOARD_TFT_DATA6,
                BOARD_TFT_DATA7,
            },
        .bus_width = 8,
        .max_transfer_bytes = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
        .psram_trans_align = 64,
        .sram_trans_align = 4};
    // esp_lcd_new_i80_bus(&lcd_bus_config, &i80_bus);
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&lcd_bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t lcd_io_config = {
        .cs_gpio_num = BOARD_TFT_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .on_color_trans_done = panel_io_trans_done_callback,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels =
            {
                .dc_idle_level = 0,
                .dc_cmd_level = 0,
                .dc_dummy_level = 0,
                .dc_data_level = 1,
            },
        .flags =
            {
                .cs_active_high = 0,
                .reverse_color_bits = 0,
                .swap_color_bytes = 1,
                .pclk_active_neg = 0,
                .pclk_idle_low = 0,
            },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &lcd_io_config, &lcd_io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BOARD_TFT_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        .bits_per_pixel = 16,
        .flags =
            {
                .reset_active_high = 1,
            },
        .vendor_config = NULL,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_io_handle, &panel_config, &panel_handle));
    ESP_LOGI("LVGL", "ST7789 panel created");
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_LOGI("LVGL", "ST7789 panel reset done");
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 0));
    ESP_LOGI("LVGL", "ST7789 panel gap set");
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_LOGI("LVGL", "ST7789 panel initialized");
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, false));
    ESP_LOGI("LVGL", "ST7789 panel color inversion set");
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, true));
    ESP_LOGI("LVGL", "ST7789 panel mirror set");
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_LOGI("LVGL", "ST7789 panel swap xy set %bool", true);
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    ESP_LOGI("LVGL", "ST7789 panel display on");

    //  Configurare SPI Touch IO
    spi_bus_config_t buscfg = {.mosi_io_num = (int)PIN_NUM_MOSI,
        .miso_io_num = (int)PIN_NUM_MISO,
        .sclk_io_num = (int)PIN_NUM_CLK,
        .quadwp_io_num = (int)(-1),
        .quadhd_io_num = (int)(-1),
        .data4_io_num = (int)(-1),
        .data5_io_num = (int)(-1),
        .data6_io_num = (int)(-1),
        .data7_io_num = (int)(-1),
        .data_io_default_level = 0,
        .max_transfer_sz = (int)4096,
        .flags = 0,
        .isr_cpu_id = (esp_intr_cpu_affinity_t)0,
        .intr_flags = 0};
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI("LVGL", "SPI bus initialized");

    // Configurare IO pentru touch
    esp_lcd_panel_io_spi_config_t touch_io_config = {.cs_gpio_num = (gpio_num_t)PIN_NUM_CS,
        .dc_gpio_num = GPIO_NUM_NC,
        .spi_mode = 0,
        .pclk_hz = ESP_LCD_TOUCH_SPI_CLOCK_HZ,
        .trans_queue_depth = 3,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .flags = {.dc_high_on_cmd = 0,
            .dc_low_on_data = 0,
            .dc_low_on_param = 0,
            .octal_mode = 0,
            .quad_mode = 0,
            .sio_mode = 0,
            .lsb_first = 0,
            .cs_high_active = 0}};
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &touch_io_config, &touch_io_handle));
    ESP_LOGI("LVGL", "Touch panel IO created");

    // Configurare driver touch
    esp_lcd_touch_config_t touch_config = {.x_max = 4095,
        .y_max = 4095,
        .rst_gpio_num = (gpio_num_t)-1,
        .int_gpio_num = (gpio_num_t)PIN_NUM_IRQ,
        .levels = {.reset = 0, .interrupt = 0},
        .flags = {.swap_xy = true, .mirror_x = false, .mirror_y = false},
        .process_coordinates = NULL,
        .interrupt_callback = NULL,
        .user_data = NULL,
        .driver_data = NULL};
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(touch_io_handle, &touch_config, &touch_handle));
    ESP_LOGI("LVGL", "Touch panel created");

    lvgl_port_cfg_t cfg = ESP_LVGL_PORT_INIT_CONFIG();
    cfg.task_stack = 16000;
    cfg.task_priority = 5;
    cfg.task_affinity = 1;
    cfg.timer_period_ms = 5;
    ESP_ERROR_CHECK(lvgl_port_init(&cfg));

    disp = lv_display_create((int32_t)LCD_WIDTH, (int32_t)LCD_HEIGHT);

    lv_display_set_flush_cb(disp, lv_disp_flush); // Set the flush callback which will be called to
                                                  // copy the rendered image to the display.
    ESP_LOGI("LVGL", "LVGL display flush callback set");

#if (BUFFER_MODE == BUFFER_FULL)
    bufSize =
        ((LCD_WIDTH * LCD_HEIGHT) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_60LINES)
    bufSize = ((LCD_WIDTH * 60) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_40LINES)
    bufSize = ((LCD_WIDTH * 40) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_20LINES)
    bufSize = ((LCD_WIDTH * 20) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_DEVIDED4)
    bufSize = ((LCD_WIDTH * LCD_HEIGHT) *
               lv_color_format_get_size(lv_display_get_color_format(disp)) / 4);
#endif
#if (BUFFER_MEM == BUFFER_SPIRAM)
#if (DOUBLE_BUFFER_MODE == 1)
    disp_draw_buf = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
    disp_draw_buf_II = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
    ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#else
    disp_draw_buf = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
#endif
#elif (BUFFER_MEM == BUFFER_INTERNAL)
#if (DMA_ON == 1)
#if (DOUBLE_BUFFER_MODE == 1)
    disp_draw_buf = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    disp_draw_buf_II = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#else
    disp_draw_buf = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
#endif
#else
#if (DOUBLE_BUFFER_MODE == 1)
    disp_draw_buf = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
    disp_draw_buf_II = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
    ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#else
    disp_draw_buf = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
#endif
#endif
#endif

    if (!disp_draw_buf) { // VERIFICA DACA PRIMUL BUFFER ESTE CREAT
        ESP_LOGE("LVGL", "LVGL disp_draw_buf allocate failed!");
    }
#if (DOUBLE_BUFFER_MODE == 1)
    if (!disp_draw_buf_II) { // VERIFICA DACA AL DOILEA BUFFER ESTE CREAT
        ESP_LOGE("LVGL", "LVGL disp_draw_buf_II allocate failed!");
    }
#endif

#if (DOUBLE_BUFFER_MODE == 1)
    lv_display_set_buffers(
        disp, disp_draw_buf, disp_draw_buf_II, bufSize, (lv_display_render_mode_t)RENDER_MODE);
    ESP_LOGI("LVGL", "LVGL buffers set");
#else
    lv_display_set_buffers(
        disp, disp_draw_buf, NULL, bufSize, (lv_display_render_mode_t)RENDER_MODE);
#endif

    // lv_display_set_resolution(disp, LCD_WIDTH, LCD_HEIGHT);
    // software lv_display_set_physical_resolution(disp, LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_rotation(disp, (lv_display_rotation_t)0); // Seteaza rotatia
    lv_display_set_render_mode(disp, (lv_display_render_mode_t)RENDER_MODE);
    lv_display_set_antialiasing(disp, true);
    ESP_LOGI("LVGL", "LVGL display settings done");

    lv_indev_t* indev = lv_indev_create();           /*Initialize the (dummy) input device driver*/
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    ////lv_indev_set_read_cb(indev, lv_touchpad_read);    // old version
    lv_indev_set_read_cb(indev, lv_touchpad_read_v2);
    ESP_LOGI("LVGL", "LVGL Setup done");

    lvgl_port_lock(0);
    create_tabs_ui(); // Creeaza interfata grafica
    lvgl_port_unlock();

    // start_resource_monitor();

    // Use the ftm component
    // the ftm component use {} for the parameter enter point
    fmt::print("Hello, world from <ftm> Component!\n");
    std::string string1 = fmt::format("The nr is {}.\n", 1);
    fmt::print("{}", string1);
    std::string string2 = fmt::format("I'd rather be {1} than {0}.\n", "right", "happy");
    fmt::print("{}\n", string2);
    std::string string3 = fmt::format("{1}{0}\n", "Hello", "World");
    fmt::print("{}\n", string3);
    fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "Hello, {}!\n", "world");
    fmt::print(fg(fmt::color::floral_white) | bg(fmt::color::slate_gray) | fmt::emphasis::underline,"Olá, {}!\n", "Mundo");
    fmt::print(fg(fmt::color::steel_blue) | fmt::emphasis::italic, "Hello{}！\n", "World");

    StartCLI();

    xTaskCreatePinnedToCore(chechButton0State, // Functia care ruleaza task-ul
        (const char*)"v_check_0_pin_state",    // Numele task-ului
        (uint32_t)(4096),                      // Dimensiunea stack-ului
        (NULL),                                // Parametri
        (UBaseType_t)configMAX_PRIORITIES - 7, // Prioritatea task-ului // 6
        &xHandle_chechButton0State,            // Handle-ul task-ului
        ((1))                                  // Nucleul pe care ruleaza (ESP32 e dual-core)
    );
} // app_main

/********************************************** */

/**********************
 *   END OF FILE
 **********************/
