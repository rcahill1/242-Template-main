#include "lcd.h"
#include "liblvgl/core/lv_disp.h"
#include "liblvgl/core/lv_obj.h"
#include "liblvgl/core/lv_obj_pos.h"
#include "liblvgl/core/lv_obj_style.h"
#include "liblvgl/lvgl.h" // IWYU pragma: keep
#include "liblvgl/misc/lv_color.h"
#include "liblvgl/misc/lv_style.h"
#include "liblvgl/widgets/lv_label.h"
#include <string>

// screen data
lv_obj_t * screen = nullptr;
bool is_init = false;
std::string lines[15];

void render_text() {
    std::string msg = "";
    for (std::string l : lines) {
        msg.append(l);
        msg.append("\n");
    }

    lv_label_set_text(screen, msg.c_str());
}

bool lcd::initialize(void) {
    if (is_init) return false;

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, 0);
    lv_style_set_text_color(&style, lv_color_white());

    screen = lv_label_create(lv_scr_act());
    lv_obj_set_size(screen, 480, 240);
    lv_obj_add_style(screen, &style, 0);
    
    render_text();

    is_init = true;
    return true;
}

bool lcd::is_initialized(void) {
    // return wether or not the lcd screen has been created
    return is_init;
}

bool lcd::print(std::int16_t line, std::string text) {
    if (!is_init) return false;

    if (!(line >= 0 && line <= 14)) return false;

    lines[line] = text;

    render_text();

    // update text on the lcd
    return true;
}

bool lcd::clear_line(std::int16_t line) {
    if (!is_init) return false;

    if (!(line >= 0 && line <= 14)) return false;

    lines[line] = "";

    render_text();

    // clear specific lines on the screen
    return true;
}

bool lcd::clear() {
    if (!is_init) return false;

    for (int i = 0; i < 15; i++) {
        lines[i] = "";
    }

    render_text();

    return true;
}

// other methods to come