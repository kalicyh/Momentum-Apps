#include "../can_commander.h"

typedef enum {
    ObdPidListRpm = 0,
    ObdPidListSpeed,
    ObdPidListCoolant,
    ObdPidListThrottle,
    ObdPidListLoad,
    ObdPidListFuel,
    ObdPidListIat,
    ObdPidListBaro,
    ObdPidListOdometer,
} ObdPidListIndex;

static void cancommander_scene_obd_pid_list_menu_callback(void* context, uint32_t index) {
    App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void cancommander_scene_obd_pid_set_pid(App* app, const char* pid_token, const char* label) {
    app_args_set_key_value(app->args_obd_pid, sizeof(app->args_obd_pid), "pid", pid_token);
    app_set_status(app, CAN_COMMANDER_UI_TEXT("OBD PID selected: %s", "已选择 OBD PID: %s"), label);
}

void cancommander_scene_obd_pid_list_menu_on_enter(void* context) {
    App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, CAN_COMMANDER_UI_TEXT("PID List", "PID 列表"));

    submenu_add_item(app->submenu, "RPM", ObdPidListRpm, cancommander_scene_obd_pid_list_menu_callback, app);
    submenu_add_item(
        app->submenu, CAN_COMMANDER_UI_TEXT("Speed", "车速"), ObdPidListSpeed, cancommander_scene_obd_pid_list_menu_callback, app);
    submenu_add_item(
        app->submenu, CAN_COMMANDER_UI_TEXT("Coolant Temp", "冷却液温度"), ObdPidListCoolant, cancommander_scene_obd_pid_list_menu_callback, app);
    submenu_add_item(
        app->submenu,
        CAN_COMMANDER_UI_TEXT("Throttle Position", "节气门位置"),
        ObdPidListThrottle,
        cancommander_scene_obd_pid_list_menu_callback,
        app);
    submenu_add_item(
        app->submenu, CAN_COMMANDER_UI_TEXT("Engine Load", "发动机负载"), ObdPidListLoad, cancommander_scene_obd_pid_list_menu_callback, app);
    submenu_add_item(
        app->submenu, CAN_COMMANDER_UI_TEXT("Fuel Level", "燃油液位"), ObdPidListFuel, cancommander_scene_obd_pid_list_menu_callback, app);
    submenu_add_item(
        app->submenu, CAN_COMMANDER_UI_TEXT("Intake Temp", "进气温度"), ObdPidListIat, cancommander_scene_obd_pid_list_menu_callback, app);
    submenu_add_item(
        app->submenu, CAN_COMMANDER_UI_TEXT("Barometric", "大气压力"), ObdPidListBaro, cancommander_scene_obd_pid_list_menu_callback, app);
    submenu_add_item(
        app->submenu, CAN_COMMANDER_UI_TEXT("Odometer", "里程表"), ObdPidListOdometer, cancommander_scene_obd_pid_list_menu_callback, app);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, cancommander_scene_obd_pid_list_menu));

    view_dispatcher_switch_to_view(app->view_dispatcher, AppViewSubmenu);
}

bool cancommander_scene_obd_pid_list_menu_on_event(void* context, SceneManagerEvent event) {
    App* app = context;

    if(event.type != SceneManagerEventTypeCustom) {
        return false;
    }

    scene_manager_set_scene_state(app->scene_manager, cancommander_scene_obd_pid_list_menu, event.event);

    switch(event.event) {
    case ObdPidListRpm:
        cancommander_scene_obd_pid_set_pid(app, "0C", "RPM");
        break;
    case ObdPidListSpeed:
        cancommander_scene_obd_pid_set_pid(app, "0D", "Speed");
        break;
    case ObdPidListCoolant:
        cancommander_scene_obd_pid_set_pid(app, "05", "Coolant");
        break;
    case ObdPidListThrottle:
        cancommander_scene_obd_pid_set_pid(app, "11", "Throttle");
        break;
    case ObdPidListLoad:
        cancommander_scene_obd_pid_set_pid(app, "04", "Load");
        break;
    case ObdPidListFuel:
        cancommander_scene_obd_pid_set_pid(app, "2F", "Fuel");
        break;
    case ObdPidListIat:
        cancommander_scene_obd_pid_set_pid(app, "0F", "IAT");
        break;
    case ObdPidListBaro:
        cancommander_scene_obd_pid_set_pid(app, "33", "Baro");
        break;
    case ObdPidListOdometer:
        cancommander_scene_obd_pid_set_pid(app, "A6", "Odometer");
        break;
    default:
        return false;
    }

    app_action_tool_start(app, CcToolObdPid, app->args_obd_pid, "obd_pid");
    scene_manager_next_scene(
        app->scene_manager,
        app->connected ? cancommander_scene_monitor : cancommander_scene_status);
    return true;
}

void cancommander_scene_obd_pid_list_menu_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}
