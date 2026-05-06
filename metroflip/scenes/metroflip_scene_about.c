#include "../metroflip_i.h"
#include <dolphin/dolphin.h>
#include "../api/metroflip/metroflip_api.h"

#define TAG "Metroflip:Scene:About"

void metroflip_scene_about_on_enter(void* context) {
    Metroflip* app = context;
    Widget* widget = app->widget;

    dolphin_deed(DolphinDeedNfcReadSuccess);
    furi_string_reset(app->text_box_store);

    FuriString* str = furi_string_alloc();

    furi_string_printf(str, METROFLIP_UI_TEXT("\e#About (Ver: 1.0):\n\n", "关于 (版本: 1.0):\n\n"));
    furi_string_cat_printf(
        str,
        METROFLIP_UI_TEXT(
            "Metroflip is a multi-protocol metro card reader app for the Flipper Zero, created by luu176, inspired by the Metrodroid project. It enables the parsing and analysis of metro cards from transit systems around the world, providing a proof-of-concept for exploring transit card data in a portable format.",
            "Metroflip 是 Flipper Zero 的多协议地铁卡读取器，由 luu176 创建，受 Metrodroid 项目启发。它能解析和分析世界各地的地铁卡，为便携探索交通卡数据提供概念验证。"));

    widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(str));

    widget_add_button_element(
        widget, GuiButtonTypeRight, METROFLIP_UI_TEXT("Exit", "退出"), metroflip_exit_widget_callback, app);

    furi_string_free(str);
    view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewWidget);
}

bool metroflip_scene_about_on_event(void* context, SceneManagerEvent event) {
    Metroflip* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(app->scene_manager);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, MetroflipSceneStart);
        consumed = true;
    }
    return consumed;
}

void metroflip_scene_about_on_exit(void* context) {
    Metroflip* app = context;
    widget_reset(app->widget);
}
