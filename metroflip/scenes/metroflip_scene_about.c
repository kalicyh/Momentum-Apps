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

    furi_string_printf(str, METROFLIP_UI_TEXT("\e#About (Ver: 1.0):\n\n", "\xe5\x85\xb3\xe4\xba\x8e (\xe7\x89\x88\xe6\x9c\xac: 1.0):\n\n"));
    furi_string_cat_printf(
        str,
        METROFLIP_UI_TEXT(
            "Metroflip is a multi-protocol metro card reader app for the Flipper Zero, created by luu176, inspired by the Metrodroid project. It enables the parsing and analysis of metro cards from transit systems around the world, providing a proof-of-concept for exploring transit card data in a portable format.",
            "Metroflip \xe6\x98\xaf Flipper Zero \xe7\x9a\x84\xe5\xa4\x9a\xe5\x8d\x8f\xe8\xae\xae\xe5\x9c\xb0\xe9\x93\x81\xe5\x8d\xa1\xe8\xaf\xbb\xe5\x8f\x96\xe5\x99\xa8\xef\xbc\x8c\xe7\x94\xb1 luu176 \xe5\x88\x9b\xe5\xbb\xba\xef\xbc\x8c\xe5\x8f\x97 Metrodroid \xe9\xa1\xb9\xe7\x9b\xae\xe5\x90\xaf\xe5\x8f\x91\xe3\x80\x82\xe5\xae\x83\xe8\x83\xbd\xe8\xa7\xa3\xe6\x9e\x90\xe5\x92\x8c\xe5\x88\x86\xe6\x9e\x90\xe4\xb8\x96\xe7\x95\x8c\xe5\x90\x84\xe5\x9c\xb0\xe7\x9a\x84\xe5\x9c\xb0\xe9\x93\x81\xe5\x8d\xa1\xef\xbc\x8c\xe4\xb8\xba\xe4\xbe\xbf\xe6\x90\xba\xe6\x8e\xa2\xe7\xb4\xa2\xe4\xba\xa4\xe9\x80\x9a\xe5\x8d\xa1\xe6\x95\xb0\xe6\x8d\xae\xe6\x8f\x90\xe4\xbe\x9b\xe6\xa6\x82\xe5\xbf\xb5\xe9\xaa\x8c\xe8\xaf\x81\xe3\x80\x82"));

    widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(str));

    widget_add_button_element(
        widget, GuiButtonTypeRight, METROFLIP_UI_TEXT("Exit", "\xe9\x80\x80\xe5\x87\xba"), metroflip_exit_widget_callback, app);

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
