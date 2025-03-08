#include "user_interface.h"

static const char *TAG = "LVGL";


/*********************************************************
 * Screen 0
 * - displays sips count and refill count
 *
 * Screen 1
 * - displays count down time for the next sip
 * - includes a label and a bar
 *
 * Screen 2
 * - Drink reminder
 *
 * Screen 3
 * - Replace glass
 *********************************************************/

// screen 0
lv_obj_t *scr_0_label_0 = NULL;
lv_obj_t *scr_0_label_1 = NULL;

// screen 1
lv_obj_t *scr_1_label_0 = NULL;
lv_obj_t *scr_1_label_1 = NULL;
lv_obj_t *scr_1_bar_0 = NULL;

// screen 2
lv_obj_t *scr_2_label_0 = NULL;

// screen objects
lv_obj_t *scr[UI_SCREEN_NUM];

// stores the currently active screen
uint32_t ui_active_screen = 1;

uint32_t ui_get_active_screen_num(void){
    return ui_active_screen;
}

// directly sets the screen number we want
void ui_set_active_screen(uint32_t screen_number){
    if(screen_number < UI_SCREEN_NUM){
        if (lvgl_port_lock(0)) {
            if (scr[screen_number] != NULL) {
                ui_active_screen = screen_number;
                lv_scr_load_anim(scr[ui_active_screen], LV_SCR_LOAD_ANIM_OUT_TOP, 400, 0, false);
            }
            lvgl_port_unlock();
        }
    }
}

// switches to the following screen
// returns the currently active screen
uint32_t ui_advance_screen(void){
    ui_active_screen++;

    // skip the enjoy screen
    if(ui_active_screen == UI_SCREEN_SPLASH)
        ui_active_screen++;

    // roll over if necessary
    if(ui_active_screen >= UI_SCREEN_NUM){
        ui_active_screen = 0;
    }

    // set the active screen
    ui_set_active_screen(ui_active_screen);

    return ui_active_screen;
}

void ui_set_scr_0_label_0_text(const char *text) {
    if (lvgl_port_lock(0)) {
        if (scr_0_label_0 != NULL) {
            lv_label_set_text(scr_0_label_0, text);
        }
        lvgl_port_unlock();
    }
}

void ui_set_scr_0_label_1_text(const char *text) {
    if (lvgl_port_lock(0)) {
        if (scr_0_label_1 != NULL) {
            lv_label_set_text(scr_0_label_1, text);
        }
        lvgl_port_unlock();
    }
}

void ui_set_scr_1_label_0_text(const char *text) {
    if (lvgl_port_lock(0)) {
        if (scr_1_label_0 != NULL) {
            lv_label_set_text(scr_1_label_0, text);
        }
        lvgl_port_unlock();
    }
}

void ui_set_scr_1_label_1_text(const char *text) {
    if (lvgl_port_lock(0)) {
        if (scr_1_label_1 != NULL) {
            lv_label_set_text(scr_1_label_1, text);
        }
        lvgl_port_unlock();
    }
}

void ui_set_scr_1_bar_0_value(int32_t value) {
    if (lvgl_port_lock(0)) {
        if (scr_1_bar_0 != NULL) {
            lv_bar_set_value(scr_1_bar_0, value, LV_ANIM_ON);
        }
        lvgl_port_unlock();
    }
}


void ui_init(lv_display_t *disp)
{
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(0)) {

        // Create screens
        for (int i = 0; i < UI_SCREEN_NUM; i++) {
            scr[i] = lv_obj_create(NULL);
        }

        // screen 0 definition
        scr_0_label_0 = lv_label_create(scr[0]);
        lv_label_set_long_mode(scr_0_label_0, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_label_set_text(scr_0_label_0, "StuckAtPrototype");
        lv_obj_set_width(scr_0_label_0, 72); // width of our screen
        lv_obj_align(scr_0_label_0, LV_ALIGN_TOP_MID, 0, 0);

        scr_0_label_1 = lv_label_create(scr[0]);
        lv_label_set_long_mode(scr_0_label_1, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_label_set_text(scr_0_label_1, "Youtube!");
        lv_obj_set_width(scr_0_label_1, 72); // width of our screen
        lv_obj_align(scr_0_label_1, LV_ALIGN_TOP_MID, 0, 20);


        // screen 1 definition
        scr_1_label_0 = lv_label_create(scr[1]);
        lv_label_set_text(scr_1_label_0, "30");
        lv_obj_set_width(scr_1_label_0, 24); // width of our screen
        lv_label_set_long_mode(scr_1_label_0, LV_LABEL_LONG_SCROLL_CIRCULAR);
//        lv_obj_set_style_text_align(scr_1_label_0, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(scr_1_label_0, LV_ALIGN_TOP_LEFT, 28, 0);

        scr_1_bar_0 = lv_bar_create(scr[1]);
        lv_obj_set_size(scr_1_bar_0, 40, 8);
        lv_obj_align(scr_1_bar_0, LV_ALIGN_TOP_LEFT, 54, 4);
        lv_bar_set_value(scr_1_bar_0, 100, LV_ANIM_ON);
        lv_obj_set_style_bg_color(scr_1_bar_0, lv_color_black(), LV_PART_INDICATOR);

        scr_1_label_1 = lv_label_create(scr[1]);
        lv_label_set_long_mode(scr_1_label_1, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_label_set_text(scr_1_label_1, "Youtube!");
        lv_obj_set_width(scr_1_label_1, 72); // width of our screen
        lv_obj_align(scr_1_label_1, LV_ALIGN_TOP_LEFT, 28, 20);

        // screen 2 definition
        scr_2_label_0 = lv_label_create(scr[2]);
        lv_label_set_text(scr_2_label_0, "Enjoy!");
        lv_obj_set_width(scr_2_label_0, 72); // width of our screen
        lv_obj_set_style_text_align(scr_2_label_0, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(scr_2_label_0, LV_ALIGN_TOP_MID, 0, 10);

        lv_scr_load_anim(scr[ui_active_screen], LV_SCR_LOAD_ANIM_OUT_TOP, 400, 0, false);

    }
    lvgl_port_unlock();
}