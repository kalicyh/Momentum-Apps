#include "../mass_storage_app_i.h"

void mass_storage_scene_usb_locked_on_enter(void* context) {
    MassStorageApp* app = context;

    widget_add_icon_element(app->widget, 78, 0, &I_ActiveConnection_50x64);
    widget_add_string_multiline_element(
        app->widget, 3, 2, AlignLeft, AlignTop, FontPrimary, MASS_STORAGE_UI_TEXT("Connection\nis active!", "连接\n已激活!"));
    widget_add_string_multiline_element(
        app->widget,
        3,
        30,
        AlignLeft,
        AlignTop,
        FontSecondary,
        MASS_STORAGE_UI_TEXT("Disconnect from\nPC or phone to\nuse this function.", "请断开电脑或\n手机连接后\n再使用此功能."));

    view_dispatcher_switch_to_view(app->view_dispatcher, MassStorageAppViewWidget);
}

bool mass_storage_scene_usb_locked_on_event(void* context, SceneManagerEvent event) {
    MassStorageApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, MassStorageSceneFileSelect);
        if(!consumed) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, MassStorageSceneStart);
        }
    }

    return consumed;
}

void mass_storage_scene_usb_locked_on_exit(void* context) {
    MassStorageApp* app = context;
    widget_reset(app->widget);
}
