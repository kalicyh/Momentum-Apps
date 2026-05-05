#include "../mag_i.h"

#ifndef MAGSPOOF_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define MAGSPOOF_UI_TEXT(en, zh) (zh)
#else
#define MAGSPOOF_UI_TEXT(en, zh) (en)
#endif
#endif

void mag_scene_under_construction_on_enter(void* context) {
    Mag* mag = context;
    Widget* widget = mag->widget;

    FuriString* tmp_str;
    tmp_str = furi_string_alloc();

    widget_add_button_element(
        widget, GuiButtonTypeLeft, MAGSPOOF_UI_TEXT("Back", "返回"), mag_widget_callback, mag);

    furi_string_printf(tmp_str, "%s", MAGSPOOF_UI_TEXT("Under construction!", "建设中!"));
    widget_add_string_element(
        widget, 64, 4, AlignCenter, AlignTop, FontPrimary, furi_string_get_cstr(tmp_str));
    furi_string_reset(tmp_str);

    view_dispatcher_switch_to_view(mag->view_dispatcher, MagViewWidget);
    furi_string_free(tmp_str);
}

bool mag_scene_under_construction_on_event(void* context, SceneManagerEvent event) {
    Mag* mag = context;
    SceneManager* scene_manager = mag->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = true;

            scene_manager_previous_scene(scene_manager);
        }
    }

    return consumed;
}

void mag_scene_under_construction_on_exit(void* context) {
    Mag* mag = context;
    widget_reset(mag->widget);
}
