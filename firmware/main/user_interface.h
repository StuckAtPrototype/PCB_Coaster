#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "lvgl.h"
#include "esp_lvgl_port.h"

#define UI_SCREEN_NUM 3

#define UI_SCREEN_SPLASH 2

void ui_init(lv_display_t *disp);

// returns the active screen number
uint32_t ui_get_active_screen_num(void);
uint32_t ui_advance_screen(void);
void ui_set_active_screen(uint32_t screen_number);

// setters
void ui_set_scr_0_label_0_text(const char *text);
void ui_set_scr_0_label_1_text(const char *text);
void ui_set_scr_1_label_0_text(const char *text);
void ui_set_scr_1_label_1_text(const char *text);
void ui_set_scr_1_bar_0_value(int32_t value);

#endif //USERINTERFACE_H
