#include "../can_commander.h"

typedef enum {
    DbcClear = 0,
    DbcAdd,
    DbcRemove,
    DbcList,
    DbcSaveConfig,
} DbcMenuIndex;

static void cancommander_scene_dbc_menu_callback(void* context, uint32_t index) {
    App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void cancommander_scene_dbc_add_apply(App* app) {
    app_action_dbc_add(app, app->args_dbc_add);
}

static void cancommander_scene_dbc_remove_apply(App* app) {
    app_action_dbc_remove(app, app->args_dbc_remove);
}

void cancommander_scene_dbc_menu_on_enter(void* context) {
    App* app = context;

    submenu_reset(app->submenu);

    submenu_add_item(
        app->submenu,
        CAN_COMMANDER_UI_TEXT("DBC Clear", "清空 DBC"),
        DbcClear,
        cancommander_scene_dbc_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        CAN_COMMANDER_UI_TEXT("DBC Add", "添加 DBC"),
        DbcAdd,
        cancommander_scene_dbc_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        CAN_COMMANDER_UI_TEXT("DBC Remove", "移除 DBC"),
        DbcRemove,
        cancommander_scene_dbc_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        CAN_COMMANDER_UI_TEXT("DBC List", "DBC 列表"),
        DbcList,
        cancommander_scene_dbc_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        CAN_COMMANDER_UI_TEXT("Save DBC Profile", "保存 DBC 配置"),
        DbcSaveConfig,
        cancommander_scene_dbc_menu_callback,
        app);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, cancommander_scene_dbc_menu));

    view_dispatcher_switch_to_view(app->view_dispatcher, AppViewSubmenu);
}

bool cancommander_scene_dbc_menu_on_event(void* context, SceneManagerEvent event) {
    App* app = context;

    if(event.type != SceneManagerEventTypeCustom) {
        return false;
    }

    scene_manager_set_scene_state(app->scene_manager, cancommander_scene_dbc_menu, event.event);

    switch(event.event) {
    case DbcClear:
        app_action_dbc_clear(app);
        break;

    case DbcAdd:
        app_begin_args_editor_apply(
            app,
            app->args_dbc_add,
            sizeof(app->args_dbc_add),
            CAN_COMMANDER_UI_TEXT("DBC Add", "DBC 添加"),
            CAN_COMMANDER_UI_TEXT("Add", "添加"),
            cancommander_scene_dbc_add_apply,
            cancommander_scene_status);
        scene_manager_next_scene(app->scene_manager, cancommander_scene_args_editor);
        return true;

    case DbcRemove:
        app_begin_args_editor_apply(
            app,
            app->args_dbc_remove,
            sizeof(app->args_dbc_remove),
            CAN_COMMANDER_UI_TEXT("DBC Remove", "DBC 移除"),
            CAN_COMMANDER_UI_TEXT("Remove", "移除"),
            cancommander_scene_dbc_remove_apply,
            cancommander_scene_status);
        scene_manager_next_scene(app->scene_manager, cancommander_scene_args_editor);
        return true;

    case DbcList:
        app_action_dbc_list(app);
        break;

    case DbcSaveConfig:
        scene_manager_next_scene(app->scene_manager, cancommander_scene_dbc_save_config_menu);
        return true;

    default:
        return false;
    }

    scene_manager_next_scene(app->scene_manager, cancommander_scene_status);
    return true;
}

void cancommander_scene_dbc_menu_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}
