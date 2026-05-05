#include "../weebo_i.h"
#include <dolphin/dolphin.h>

#define TAG "WeeboSceneInfo"

void weebo_scene_info_on_enter(void* context) {
    Weebo* weebo = context;

    furi_string_reset(weebo->text_box_store);
    FuriString* str = weebo->text_box_store;
    FuriString* name = furi_string_alloc();

    furi_string_cat_printf(str, WEEBO_UI_TEXT("Info:\n", "信息:\n"));
    if(weebo_get_figure_name(weebo, name)) {
        furi_string_cat_printf(str, "%s\n", furi_string_get_cstr(name));
    } else {
        furi_string_cat_printf(str, WEEBO_UI_TEXT("Unknown\n", "未知\n"));
    }
    furi_string_cat_printf(str, WEEBO_UI_TEXT("ID: %04x\n", "编号: %04x\n"), weebo_get_figure_id(weebo));
    if(weebo_get_figure_form(weebo, name)) {
        furi_string_cat_printf(str, WEEBO_UI_TEXT("Form: %s\n", "形态: %s\n"), furi_string_get_cstr(name));
    }
    if(weebo_get_figure_series(weebo, name)) {
        furi_string_cat_printf(str, WEEBO_UI_TEXT("Series: %s\n", "系列: %s\n"), furi_string_get_cstr(name));
    }

    furi_string_free(name);
    text_box_set_font(weebo->text_box, TextBoxFontText);
    text_box_set_text(weebo->text_box, furi_string_get_cstr(weebo->text_box_store));
    view_dispatcher_switch_to_view(weebo->view_dispatcher, WeeboViewTextBox);
}

bool weebo_scene_info_on_event(void* context, SceneManagerEvent event) {
    Weebo* weebo = context;
    bool consumed = false;
    UNUSED(weebo);

    if(event.type == SceneManagerEventTypeCustom) {
    }
    return consumed;
}

void weebo_scene_info_on_exit(void* context) {
    Weebo* weebo = context;

    // Clear view
    text_box_reset(weebo->text_box);
}
