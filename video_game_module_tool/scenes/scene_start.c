#include "app_i.h"

#include <furi.h>

typedef enum {
    SceneStartIndexInstallDefault,
    SceneStartIndexInstallRGB,
    SceneStartIndexInstallCustom,
} SceneStartIndex;

void scene_start_on_enter(void* context) {
    App* app = context;

    if(!furi_string_empty(app->file_path)) {
        // File path is set, go directly to firmware install
        scene_manager_next_scene(app->scene_manager, SceneInstall);
        return;
    }

    submenu_add_item(
        app->submenu,
        VGM_UI_TEXT("Install Official Firmware", "安装官方固件"),
        SceneStartIndexInstallDefault,
        submenu_item_common_callback,
        app);
    submenu_add_item(
        app->submenu,
        VGM_UI_TEXT("Install RGB Firmware", "安装RGB固件"),
        SceneStartIndexInstallRGB,
        submenu_item_common_callback,
        app);
    submenu_add_item(
        app->submenu,
        VGM_UI_TEXT("Install Firmware from File", "从文件安装固件"),
        SceneStartIndexInstallCustom,
        submenu_item_common_callback,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIdSubmenu);
}

bool scene_start_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    App* app = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SceneStartIndexInstallDefault) {
            furi_string_set(app->file_path, VGM_DEFAULT_FW_FILE);
            scene_manager_next_scene(app->scene_manager, SceneConfirm);
        } else if(event.event == SceneStartIndexInstallRGB) {
            furi_string_set(app->file_path, APP_ASSETS_PATH("vgm-fw-rgb-0.1.0.uf2"));
            scene_manager_next_scene(app->scene_manager, SceneConfirm);
        } else if(event.event == SceneStartIndexInstallCustom) {
            scene_manager_next_scene(app->scene_manager, SceneFileSelect);
        }

        return true;
    } else if(event.type == SceneManagerEventTypeBack) {
        view_dispatcher_stop(app->view_dispatcher);
        return true;
    }

    return false;
}

void scene_start_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}
