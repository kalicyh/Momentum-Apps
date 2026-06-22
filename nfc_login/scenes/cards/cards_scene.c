#include "cards_scene.h"

void app_render_edit_menu(App* app) {
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, NFC_LOGIN_UI_TEXT("Edit Card", "编辑卡片"));
    const char* items[] = {
        NFC_LOGIN_UI_TEXT("Name", "名称"),
        NFC_LOGIN_UI_TEXT("Password", "密码"),
        NFC_LOGIN_UI_TEXT("UID", "UID"),
        NFC_LOGIN_UI_TEXT("Delete", "删除")};
    for(size_t i = 0; i < 4; i++) {
        char line[32];
        snprintf(line, sizeof(line), "%s %s", (i == app->edit_menu_index) ? ">" : " ", items[i]);
        widget_add_string_element(app->widget, 0, 12 + i * 12, AlignLeft, AlignTop, FontSecondary, line);
    }
}

void app_render_card_list(App* app) {
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, NFC_LOGIN_UI_TEXT("Cards", "卡片"));
    if(app->card_count == 0) {
        widget_add_string_element(app->widget, 0, 20, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("No cards stored", "暂无卡片"));
    } else {
        for(uint8_t i = 0; i < CARD_LIST_VISIBLE_ITEMS; i++) {
            size_t card_index = app->card_list_scroll_offset + i;
            if(card_index < app->card_count) {
                char line[64];
                const char* nav_marker = (card_index == app->selected_card) ? ">" : " ";
                const char* active_marker = (app->has_active_selection && card_index == app->active_card_index) ? "*" : " ";
                snprintf(line, sizeof(line), "%s%s %zu. %s",
                         nav_marker, active_marker, card_index + 1, app->cards[card_index].name);
                widget_add_string_element(app->widget, 0, 12 + i * 10, AlignLeft, AlignTop, FontSecondary, line);
            }
        }
    }
    widget_add_string_element(app->widget, 0, 54, AlignLeft, AlignTop, FontSecondary, NFC_LOGIN_UI_TEXT("OK=Sel  Hold OK=Edit  >Import", "OK=选 长按 OK=编 >导入"));
}
