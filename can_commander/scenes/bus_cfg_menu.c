#include "../can_commander.h"

typedef enum {
    BusCfgCan0 = 0,
    BusCfgCan1,
    BusCfgGetCan0,
    BusCfgGetCan1,
} BusCfgMenuIndex;

static void cancommander_scene_bus_cfg_menu_callback(void* context, uint32_t index) {
    App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void cancommander_scene_bus_cfg_apply_can0(App* app) {
    app_action_bus_set_cfg(app, CcBusCan0, app->args_bus_cfg_can0);
}

static void cancommander_scene_bus_cfg_apply_can1(App* app) {
    app_action_bus_set_cfg(app, CcBusCan1, app->args_bus_cfg_can1);
}

void cancommander_scene_bus_cfg_menu_on_enter(void* context) {
    App* app = context;

    submenu_reset(app->submenu);

    submenu_add_item(
        app->submenu,
        CAN_COMMANDER_UI_TEXT("CAN0 Settings", "CAN0 设置"),
        BusCfgCan0,
        cancommander_scene_bus_cfg_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        CAN_COMMANDER_UI_TEXT("CAN1 Settings", "CAN1 设置"),
        BusCfgCan1,
        cancommander_scene_bus_cfg_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        CAN_COMMANDER_UI_TEXT("Get CAN0 Config", "获取 CAN0 配置"),
        BusCfgGetCan0,
        cancommander_scene_bus_cfg_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        CAN_COMMANDER_UI_TEXT("Get CAN1 Config", "获取 CAN1 配置"),
        BusCfgGetCan1,
        cancommander_scene_bus_cfg_menu_callback,
        app);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, cancommander_scene_bus_cfg_menu));

    view_dispatcher_switch_to_view(app->view_dispatcher, AppViewSubmenu);
}

bool cancommander_scene_bus_cfg_menu_on_event(void* context, SceneManagerEvent event) {
    App* app = context;

    if(event.type != SceneManagerEventTypeCustom) {
        return false;
    }

    scene_manager_set_scene_state(app->scene_manager, cancommander_scene_bus_cfg_menu, event.event);

    switch(event.event) {
    case BusCfgCan0:
        app_begin_args_editor_apply(
            app,
            app->args_bus_cfg_can0,
            sizeof(app->args_bus_cfg_can0),
            CAN_COMMANDER_UI_TEXT("CAN0 Config", "CAN0 配置"),
            CAN_COMMANDER_UI_TEXT("Set CAN0", "设置 CAN0"),
            cancommander_scene_bus_cfg_apply_can0,
            cancommander_scene_status);
        scene_manager_next_scene(app->scene_manager, cancommander_scene_args_editor);
        return true;

    case BusCfgCan1:
        app_begin_args_editor_apply(
            app,
            app->args_bus_cfg_can1,
            sizeof(app->args_bus_cfg_can1),
            CAN_COMMANDER_UI_TEXT("CAN1 Config", "CAN1 配置"),
            CAN_COMMANDER_UI_TEXT("Set CAN1", "设置 CAN1"),
            cancommander_scene_bus_cfg_apply_can1,
            cancommander_scene_status);
        scene_manager_next_scene(app->scene_manager, cancommander_scene_args_editor);
        return true;

    case BusCfgGetCan0:
        app_action_bus_get_cfg(app, CcBusCan0);
        scene_manager_next_scene(
            app->scene_manager,
            app->connected ? cancommander_scene_monitor : cancommander_scene_status);
        return true;

    case BusCfgGetCan1:
        app_action_bus_get_cfg(app, CcBusCan1);
        scene_manager_next_scene(
            app->scene_manager,
            app->connected ? cancommander_scene_monitor : cancommander_scene_status);
        return true;

    default:
        return false;
    }
}

void cancommander_scene_bus_cfg_menu_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}
