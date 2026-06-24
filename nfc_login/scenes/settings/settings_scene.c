#include "settings_scene.h"
#include "../../crypto/nfc_login_passcode.h"
#include "../../hid/nfc_login_hid.h"
#include <gui/canvas.h>
#include <gui/view.h>
#include <gui/modules/widget.h>

#define PASSCODE_DOT_SIZE 6
#define PASSCODE_DOT_SPACING 10
#define PASSCODE_DOT_Y 24
#define PASSCODE_DOT_START_X 8

static void widget_custom_draw_callback(Canvas* canvas, void* context) {
    App* app = context;
    if(!app) return;
    
    // Only draw circles if we're in passcode mode
    if(app->widget_state == 6 || app->widget_state == 7) {
        bool is_lockscreen = (app->widget_state == 7);
        size_t button_count = count_buttons_in_sequence(app->passcode_sequence);
        size_t max_buttons = MAX_PASSCODE_BUTTONS;
        
        if(is_lockscreen) {
            // Get stored sequence to know how many buttons to expect
            char stored_sequence[MAX_PASSCODE_SEQUENCE_LEN];
            if(get_passcode_sequence(stored_sequence, sizeof(stored_sequence))) {
                max_buttons = count_buttons_in_sequence(stored_sequence);
            }
        }
        
        // Draw circles
        int start_x = PASSCODE_DOT_START_X;
        for(size_t i = 0; i < max_buttons; i++) {
            int x = start_x + (i * PASSCODE_DOT_SPACING);
            int y = PASSCODE_DOT_Y;
            
            if(i < button_count) {
                // Filled circle for entered buttons
                canvas_draw_disc(canvas, x, y, PASSCODE_DOT_SIZE / 2);
            } else {
                // Empty circle for remaining buttons
                canvas_draw_circle(canvas, x, y, PASSCODE_DOT_SIZE / 2);
            }
        }
    }
}

void app_render_credits(App* app) {
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, NFC_LOGIN_UI_TEXT("Credits", "关于"));

    if(app->credits_page == 0) {
        widget_add_string_element(app->widget, 0, 12, AlignLeft, AlignTop, FontSecondary, "NFC Login");
        widget_add_string_element(app->widget, 0, 22, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("Version: 1.0", "版本: 1.0"));
        widget_add_string_element(app->widget, 0, 32, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("Creator: Play2BReal", "作者: Play2BReal"));
        widget_add_string_element(app->widget, 0, 42, AlignLeft, AlignTop, FontSecondary, "github.com/Play2BReal");
    } else if(app->credits_page == 1) {
        widget_add_string_element(app->widget, 0, 12, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("Special Thanks To:", "特别感谢:"));
        widget_add_string_element(app->widget, 0, 22, AlignLeft, AlignTop, FontSecondary, "Equip, Tac0s, WillyJL, pr3");
        widget_add_string_element(app->widget, 0, 32, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("& The Biohacking Community!", "及生物黑客社区!"));
        widget_add_string_element(app->widget, 0, 42, AlignLeft, AlignTop, FontSecondary, "");
    }

    char page_info[32];
    snprintf(page_info, sizeof(page_info), NFC_LOGIN_UI_TEXT("Page %d/%d  <- ->=Navigate", "第 %d/%d 页  <- -> 翻页"), app->credits_page + 1, CREDITS_PAGES);
    widget_add_string_element(app->widget, 0, SETTINGS_HELP_Y_POS, AlignLeft, AlignTop, FontSecondary, page_info);
}

void app_render_passcode_dots(App* app, size_t max_buttons) {
    // Circles are drawn by the custom draw callback
    // Just ensure the widget view has the callback set
    (void)max_buttons; // Parameter kept for API compatibility
    View* widget_view = widget_get_view(app->widget);
    if(widget_view) {
        view_set_draw_callback(widget_view, widget_custom_draw_callback);
    }
}

void app_render_lockscreen(App* app) {
    // Get the stored sequence to know how many buttons to expect
    char stored_sequence[MAX_PASSCODE_SEQUENCE_LEN];
    bool has_stored = get_passcode_sequence(stored_sequence, sizeof(stored_sequence));
    
    if(!has_stored) {
        widget_add_string_element(app->widget, 0, 24, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("Error: No stored sequence", "错误: 无已存密码"));
        return;
    }
    
    // Count buttons in stored sequence
    size_t stored_button_count = count_buttons_in_sequence(stored_sequence);
    
    // Count buttons in current input
    size_t input_button_count = count_buttons_in_sequence(app->passcode_sequence);
    
    // Circles are drawn by the custom draw callback
    // Just ensure the widget view has the callback set
    View* widget_view = widget_get_view(app->widget);
    if(widget_view) {
        view_set_draw_callback(widget_view, widget_custom_draw_callback);
    }
    
    // Show progress
    char progress[32];
    snprintf(progress, sizeof(progress), "%zu / %zu", input_button_count, stored_button_count);
    
    widget_add_string_element(app->widget, 0, 36, AlignLeft, AlignTop, FontSecondary, progress);
    
    if(input_button_count >= stored_button_count) {
        widget_add_string_element(app->widget, 0, 48, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("Press OK to verify", "按 OK 验证"));
    } else {
        widget_add_string_element(app->widget, 0, 48, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("Enter passcode...", "输入密码..."));
    }
}

void app_render_settings(App* app) {
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, NFC_LOGIN_UI_TEXT("Settings", "设置"));

    char setting_lines[SETTINGS_MENU_ITEMS][64];
    char layout_display[32];
    strncpy(layout_display, app->keyboard_layout, sizeof(layout_display) - 1);
    layout_display[sizeof(layout_display) - 1] = '\0';

    snprintf(setting_lines[0], sizeof(setting_lines[0]), "%s %s: %s",
             (app->settings_menu_index == 0) ? ">" : " ", NFC_LOGIN_UI_TEXT("HID Mode", "HID 模式"), app->hid_mode == HidModeBle ? "BLE" : "USB");
    snprintf(setting_lines[1], sizeof(setting_lines[1]), "%s %s: %s",
             (app->settings_menu_index == 1) ? ">" : " ", NFC_LOGIN_UI_TEXT("Keyboard Layout", "键盘布局"), layout_display);
    snprintf(setting_lines[2], sizeof(setting_lines[2]), "%s %s: %dms",
             (app->settings_menu_index == 2) ? ">" : " ", NFC_LOGIN_UI_TEXT("Input Delay", "输入延迟"), app->input_delay_ms);
    snprintf(setting_lines[3], sizeof(setting_lines[3]), "%s %s: %s",
             (app->settings_menu_index == 3) ? ">" : " ", NFC_LOGIN_UI_TEXT("Append Enter", "追加回车"), app->append_enter ? NFC_LOGIN_UI_TEXT("ON", "开") : NFC_LOGIN_UI_TEXT("OFF", "关"));
    snprintf(setting_lines[4], sizeof(setting_lines[4]), "%s %s",
             (app->settings_menu_index == 4) ? ">" : " ", NFC_LOGIN_UI_TEXT("Reset Passcode", "重置密码"));
    snprintf(setting_lines[5], sizeof(setting_lines[5]), "%s %s: %s",
             (app->settings_menu_index == 5) ? ">" : " ", NFC_LOGIN_UI_TEXT("Disable Passcode", "禁用密码"), get_passcode_disabled() ? NFC_LOGIN_UI_TEXT("ON", "开") : NFC_LOGIN_UI_TEXT("OFF", "关"));
    snprintf(setting_lines[6], sizeof(setting_lines[6]), "%s %s",
             (app->settings_menu_index == 6) ? ">" : " ", NFC_LOGIN_UI_TEXT("Credits", "关于"));

    for(uint8_t i = 0; i < SETTINGS_VISIBLE_ITEMS; i++) {
        uint8_t item_index = app->settings_scroll_offset + i;
        if(item_index < SETTINGS_MENU_ITEMS) {
            widget_add_string_element(app->widget, 0, 12 + i * 12, AlignLeft, AlignTop, FontSecondary, setting_lines[item_index]);
        }
    }

    if(app->settings_menu_index == 0) {
        widget_add_string_element(app->widget, 0, SETTINGS_HELP_Y_POS, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("<-> Cycle  Back=Menu", "<->切换  返回=菜单"));
    } else if(app->settings_menu_index == 1) {
        widget_add_string_element(app->widget, 0, SETTINGS_HELP_Y_POS, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("<-> Cycle  OK=Sel  Back=Menu", "<->切换  OK=选  返回=菜单"));
    } else if(app->settings_menu_index == 2) {
        widget_add_string_element(app->widget, 0, SETTINGS_HELP_Y_POS, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("<-> Cycle  Back=Menu", "<->切换  返回=菜单"));
    } else if(app->settings_menu_index == 3) {
        widget_add_string_element(app->widget, 0, SETTINGS_HELP_Y_POS, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("OK=Toggle  Back=Menu", "OK=切换  返回=菜单"));
    } else if(app->settings_menu_index == 4) {
        widget_add_string_element(app->widget, 0, SETTINGS_HELP_Y_POS, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("OK=Reset  Back=Menu", "OK=重置  返回=菜单"));
    } else if(app->settings_menu_index == 5) {
        widget_add_string_element(app->widget, 0, SETTINGS_HELP_Y_POS, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("<-> Toggle  Back=Menu", "<->切换  返回=菜单"));
    } else if(app->settings_menu_index == 6) {
        widget_add_string_element(app->widget, 0, SETTINGS_HELP_Y_POS, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("OK=View  Back=Menu", "OK=查看  返回=菜单"));
    }
}
