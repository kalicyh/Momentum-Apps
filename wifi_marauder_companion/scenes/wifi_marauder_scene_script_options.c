#include "../wifi_marauder_app_i.h"

enum SubmenuIndex {
    SubmenuIndexRun,
    SubmenuIndexSettings,
    SubmenuIndexEditStages,
    SubmenuIndexSave,
    SubmenuIndexDelete
};

void wifi_marauder_scene_script_options_save_script(WifiMarauderApp* app) {
    char script_path[256];
    snprintf(
        script_path,
        sizeof(script_path),
        "%s/%s.json",
        MARAUDER_APP_FOLDER_SCRIPTS,
        app->script->name);
    wifi_marauder_script_save_json(app->storage, script_path, app->script);

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_text(message, WIFI_MARAUDER_UI_TEXT("Saved!", "已保存!"), 5, 7, AlignLeft, AlignTop);
    dialog_message_set_icon(message, &I_DolphinDone_80x58, 36, 5);
    dialog_message_set_buttons(message, NULL, WIFI_MARAUDER_UI_TEXT("Ok", "确定"), NULL);
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);
}

static void wifi_marauder_scene_script_options_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;

    switch(index) {
    case SubmenuIndexRun:
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
        break;
    case SubmenuIndexSettings:
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptSettings);
        break;
    case SubmenuIndexEditStages:
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptEdit);
        break;
    case SubmenuIndexSave:
        wifi_marauder_scene_script_options_save_script(app);
        break;
    case SubmenuIndexDelete:
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptConfirmDelete);
        break;
    }
}

void wifi_marauder_scene_script_options_on_enter(void* context) {
    WifiMarauderApp* app = context;

    // If returning after confirming script deletion
    if(app->script == NULL) {
        scene_manager_previous_scene(app->scene_manager);
        return;
    }

    Submenu* submenu = app->submenu;

    submenu_set_header(submenu, app->script->name);
    submenu_add_item(
        submenu, WIFI_MARAUDER_UI_TEXT("[>] RUN", "[>] 运行"), SubmenuIndexRun, wifi_marauder_scene_script_options_callback, app);
    submenu_add_item(
        submenu,
        WIFI_MARAUDER_UI_TEXT("[S] SETTINGS", "[S] 设置"),
        SubmenuIndexSettings,
        wifi_marauder_scene_script_options_callback,
        app);
    submenu_add_item(
        submenu,
        WIFI_MARAUDER_UI_TEXT("[+] EDIT STAGES", "[+] 编辑阶段"),
        SubmenuIndexEditStages,
        wifi_marauder_scene_script_options_callback,
        app);
    submenu_add_item(
        submenu, WIFI_MARAUDER_UI_TEXT("[*] SAVE", "[*] 保存"), SubmenuIndexSave, wifi_marauder_scene_script_options_callback, app);
    submenu_add_item(
        submenu,
        WIFI_MARAUDER_UI_TEXT("[X] DELETE", "[X] 删除"),
        SubmenuIndexDelete,
        wifi_marauder_scene_script_options_callback,
        app);

    submenu_set_selected_item(
        submenu,
        scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneScriptOptions));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewSubmenu);
}

bool wifi_marauder_scene_script_options_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        wifi_marauder_script_free(app->script);
        app->script = NULL;
    }

    return consumed;
}

void wifi_marauder_scene_script_options_on_exit(void* context) {
    WifiMarauderApp* app = context;
    submenu_reset(app->submenu);
}
