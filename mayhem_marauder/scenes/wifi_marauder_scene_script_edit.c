#include "../wifi_marauder_app_i.h"

static void wifi_marauder_scene_script_edit_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;
    WifiMarauderScriptStage* current_stage = app->script->first_stage;
    uint32_t stage_index = 0;

    while(current_stage != NULL && stage_index < index) {
        current_stage = current_stage->next_stage;
        stage_index++;
    }
    app->script_edit_selected_stage = current_stage;

    if(app->script_edit_selected_stage != NULL) {
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptEdit, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptStageEdit);
    }
}

static void wifi_marauder_scene_script_edit_add_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;
    scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptEdit, index);
    scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptStageAdd);
}

void wifi_marauder_scene_script_edit_on_enter(void* context) {
    WifiMarauderApp* app = context;
    Submenu* submenu = app->submenu;
    WifiMarauderScript* script = app->script;
    submenu_set_header(submenu, script->name);

    WifiMarauderScriptStage* current_stage = script->first_stage;
    int stage_index = 0;
    while(current_stage != NULL) {
        switch(current_stage->type) {
        case WifiMarauderScriptStageTypeScan:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Scan", "扫描"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        case WifiMarauderScriptStageTypeSelect:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Select", "选择"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        case WifiMarauderScriptStageTypeDeauth:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Deauth", "解除认证"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        case WifiMarauderScriptStageTypeProbe:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Probe", "探测"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        case WifiMarauderScriptStageTypeSniffRaw:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Sniff raw", "原始嗅探"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        case WifiMarauderScriptStageTypeSniffBeacon:
            submenu_add_item(
                submenu,
                WIFI_MARAUDER_UI_TEXT("Sniff beacon", "嗅探 Beacon"),
                stage_index,
                wifi_marauder_scene_script_edit_callback,
                app);
            break;
        case WifiMarauderScriptStageTypeSniffDeauth:
            submenu_add_item(
                submenu,
                WIFI_MARAUDER_UI_TEXT("Sniff deauth", "嗅探解除认证"),
                stage_index,
                wifi_marauder_scene_script_edit_callback,
                app);
            break;
        case WifiMarauderScriptStageTypeSniffEsp:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Sniff esp", "嗅探 ESP"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        case WifiMarauderScriptStageTypeSniffPmkid:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Sniff PMKID", "嗅探 PMKID"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        case WifiMarauderScriptStageTypeSniffPwn:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Sniff pwn", "嗅探 Pwn"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        case WifiMarauderScriptStageTypeBeaconList:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Beacon list", "Beacon 列表"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        case WifiMarauderScriptStageTypeBeaconAp:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Beacon AP", "Beacon AP"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        case WifiMarauderScriptStageTypeExec:
            submenu_add_item(
                submenu,
                WIFI_MARAUDER_UI_TEXT("Custom command", "自定义命令"),
                stage_index,
                wifi_marauder_scene_script_edit_callback,
                app);
            break;
        case WifiMarauderScriptStageTypeDelay:
            submenu_add_item(
                submenu, WIFI_MARAUDER_UI_TEXT("Delay", "延迟"), stage_index, wifi_marauder_scene_script_edit_callback, app);
            break;
        }
        current_stage = current_stage->next_stage;
        stage_index++;
    }

    submenu_add_item(
        submenu, WIFI_MARAUDER_UI_TEXT("[+] ADD STAGE", "[+] 添加阶段"), stage_index++, wifi_marauder_scene_script_edit_add_callback, app);
    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneScriptEdit));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewSubmenu);
}

bool wifi_marauder_scene_script_edit_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void wifi_marauder_scene_script_edit_on_exit(void* context) {
    WifiMarauderApp* app = context;
    submenu_reset(app->submenu);
}
