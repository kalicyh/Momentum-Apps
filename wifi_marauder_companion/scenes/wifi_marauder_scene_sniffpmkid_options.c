#include "../wifi_marauder_app_i.h"

enum SubmenuIndex {
    SubmenuIndexPassive,
    SubmenuIndexActive,
    SubmenuIndexTargetedPassive,
    SubmenuIndexTargetedActive,
    SubmenuIndexChannelPassive,
    SubmenuIndexChannelActive,
};

static void wifi_marauder_scene_sniffpmkid_options_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;

    app->is_custom_tx_string = false; // this will be set if needed by text input
    switch(index) {
    case SubmenuIndexPassive:
        app->selected_tx_string = "sniffpmkid";
        scene_manager_set_scene_state(
            app->scene_manager, WifiMarauderSceneSniffPmkidOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
        break;
    case SubmenuIndexActive:
        app->selected_tx_string = "sniffpmkid -d";
        scene_manager_set_scene_state(
            app->scene_manager, WifiMarauderSceneSniffPmkidOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
        break;
    case SubmenuIndexTargetedPassive:
        app->selected_tx_string = "sniffpmkid -l";
        scene_manager_set_scene_state(
            app->scene_manager, WifiMarauderSceneSniffPmkidOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
        break;
    case SubmenuIndexTargetedActive:
        app->selected_tx_string = "sniffpmkid -d -l";
        scene_manager_set_scene_state(
            app->scene_manager, WifiMarauderSceneSniffPmkidOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
        break;
    case SubmenuIndexChannelPassive:
        app->selected_tx_string = "sniffpmkid -c";
        scene_manager_set_scene_state(
            app->scene_manager, WifiMarauderSceneSniffPmkidOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneTextInput);
        break;
    case SubmenuIndexChannelActive:
        app->selected_tx_string = "sniffpmkid -d -c";
        scene_manager_set_scene_state(
            app->scene_manager, WifiMarauderSceneSniffPmkidOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneTextInput);
        break;
    }
}

void wifi_marauder_scene_sniffpmkid_options_on_enter(void* context) {
    WifiMarauderApp* app = context;

    Submenu* submenu = app->submenu;

    submenu_set_header(submenu, WIFI_MARAUDER_UI_TEXT("Sniff PMKID", "嗅探 PMKID"));
    submenu_add_item(
        submenu,
        WIFI_MARAUDER_UI_TEXT("Passive", "被动"),
        SubmenuIndexPassive,
        wifi_marauder_scene_sniffpmkid_options_callback,
        app);
    submenu_add_item(
        submenu,
        WIFI_MARAUDER_UI_TEXT("Active (Force Deauth)", "主动 (强制解除认证)"),
        SubmenuIndexActive,
        wifi_marauder_scene_sniffpmkid_options_callback,
        app);
    submenu_add_item(
        submenu,
        WIFI_MARAUDER_UI_TEXT("Targeted Passive (List)", "定向被动 (列表)"),
        SubmenuIndexTargetedPassive,
        wifi_marauder_scene_sniffpmkid_options_callback,
        app);
    submenu_add_item(
        submenu,
        WIFI_MARAUDER_UI_TEXT("Targeted Active (List)", "定向主动 (列表)"),
        SubmenuIndexTargetedActive,
        wifi_marauder_scene_sniffpmkid_options_callback,
        app);
    submenu_add_item(
        submenu,
        WIFI_MARAUDER_UI_TEXT("On Channel # - Passive", "指定信道 - 被动"),
        SubmenuIndexChannelPassive,
        wifi_marauder_scene_sniffpmkid_options_callback,
        app);
    submenu_add_item(
        submenu,
        WIFI_MARAUDER_UI_TEXT("On Channel # - Active", "指定信道 - 主动"),
        SubmenuIndexChannelActive,
        wifi_marauder_scene_sniffpmkid_options_callback,
        app);

    submenu_set_selected_item(
        submenu,
        scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneSniffPmkidOptions));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewSubmenu);
}

bool wifi_marauder_scene_sniffpmkid_options_on_event(void* context, SceneManagerEvent event) {
    //WifiMarauderApp* app = context;
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

void wifi_marauder_scene_sniffpmkid_options_on_exit(void* context) {
    WifiMarauderApp* app = context;
    submenu_reset(app->submenu);
}
