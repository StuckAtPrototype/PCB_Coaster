#include "screen.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"

#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "user_interface.h"

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ    (400 * 1000)
#define EXAMPLE_PIN_NUM_SDA           11 // 14
#define EXAMPLE_PIN_NUM_SCL           22 // 13
#define EXAMPLE_PIN_NUM_RST           13
#define EXAMPLE_I2C_HW_ADDR           0x3C // 0x3C
#define EXAMPLE_LCD_H_RES              128 // 128
#define EXAMPLE_LCD_V_RES              64 // 32
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8
#define I2C_BUS_PORT  0

static const char *TAG = "Screen";

void screen_init(void){

    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_config = {
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .i2c_port = I2C_BUS_PORT,
            .sda_io_num = EXAMPLE_PIN_NUM_SDA,
            .scl_io_num = EXAMPLE_PIN_NUM_SCL,
            .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
            .dev_addr = EXAMPLE_I2C_HW_ADDR,
            .scl_speed_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
            .control_phase_bytes = 1,               // According to SSD1306 datasheet
            .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,   // According to SSD1306 datasheet
            .lcd_param_bits = EXAMPLE_LCD_CMD_BITS, // According to SSD1306 datasheet
            .dc_bit_offset = 6,                     // According to SSD1306 datasheet
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
            .bits_per_pixel = 1,
            .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
    };

    esp_lcd_panel_ssd1306_config_t ssd1306_config = {
            .height = EXAMPLE_LCD_V_RES,
    };
    panel_config.vendor_config = &ssd1306_config;
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "Initialize LVGL");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    ESP_LOGI(TAG, "config LVGL");

    const lvgl_port_display_cfg_t disp_cfg = {
            .io_handle = io_handle,
            .panel_handle = panel_handle,
            .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
            .double_buffer = true,
            .hres = EXAMPLE_LCD_H_RES,
            .vres = EXAMPLE_LCD_V_RES,
            .monochrome = true,
            .rotation = {
                    .swap_xy = false,
                    .mirror_x = false,
                    .mirror_y = false,
            }
    };

    // initiate the ui
    lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);
    ui_init(disp);
}