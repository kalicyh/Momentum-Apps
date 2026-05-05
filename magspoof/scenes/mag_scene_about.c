#include "../mag_i.h"

#ifndef MAGSPOOF_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define MAGSPOOF_UI_TEXT(en, zh) (zh)
#else
#define MAGSPOOF_UI_TEXT(en, zh) (en)
#endif
#endif

void mag_scene_about_on_enter(void* context) {
    Mag* mag = context;
    Widget* widget = mag->widget;

    FuriString* tmp_str;
    tmp_str = furi_string_alloc();

    furi_string_cat_printf(
        tmp_str, "%s: %s\n", MAGSPOOF_UI_TEXT("Version", "版本"), MAG_VERSION_APP);
    furi_string_cat_printf(
        tmp_str, "%s: %s\n", MAGSPOOF_UI_TEXT("Developer", "开发者"), MAG_DEVELOPER);
    furi_string_cat_printf(tmp_str, "GitHub: %s\n\n", MAG_GITHUB);

    furi_string_cat_printf(
        tmp_str,
        "%s",
        MAGSPOOF_UI_TEXT(
            "Unfinished port of Samy Kamkar's MagSpoof. Confer GitHub for updates; in the interim, use responsibly and at your own risk.",
            "Samy Kamkar MagSpoof 的未完成移植. 请关注 GitHub 获取更新; 使用风险自负."));

    // TODO: Add credits

    widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(tmp_str));
    furi_string_free(tmp_str);

    view_dispatcher_switch_to_view(mag->view_dispatcher, MagViewWidget);
}

bool mag_scene_about_on_event(void* context, SceneManagerEvent event) {
    Mag* mag = context;
    SceneManager* scene_manager = mag->scene_manager;
    bool consumed = false;

    UNUSED(event);
    UNUSED(scene_manager);

    return consumed;
}

void mag_scene_about_on_exit(void* context) {
    Mag* mag = context;
    widget_reset(mag->widget);
}
