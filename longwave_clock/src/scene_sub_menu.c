#include "flipper.h"
#include "longwave_clock_app.h"

#include "app_state.h"
#include "scene_sub_menu.h"
#include "scenes.h"
#include "module_lights.h"

#define START_ITEM     0
#define RUN_MODE_ITEM  1
#define DATA_MODE_ITEM 2
#define DATA_PIN_ITEM  3

static char* run_mode_names[] = {LWC_UI_TEXT("demo", "演示"), "GPIO"};
static char* data_mode_names[] = {LWC_UI_TEXT("normal", "正常"), LWC_UI_TEXT("inverted", "反转")};
static char* data_pin_names[] = {"A7", "A4", "B2", "C1", "C0"};

#ifdef FW_ORIGIN_Momentum
static char* GPIO_ONLY = LWC_UI_TEXT("GPIO mode\nonly!", "仅 GPIO 模式!");
static char* run_mode_start_text[] = {LWC_UI_TEXT("Start the simulation", "开始模拟"), LWC_UI_TEXT("Start the receiver", "开始接收")};
#else
static char* start_mode_text = LWC_UI_TEXT("Start in selected mode", "以选定模式开始");
#endif

void lwc_run_mode_change_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    ProtoConfig* config = lwc_get_protocol_config(app->state);

    uint8_t index = variable_item_get_current_value_index(item);
    config->run_mode = (LWCRunMode)(index);
    variable_item_set_current_value_text(item, run_mode_names[index]);

#ifdef FW_ORIGIN_Momentum
    VariableItem* start = variable_item_list_get(app->sub_menu, START_ITEM);
    VariableItem* data_mode = variable_item_list_get(app->sub_menu, DATA_MODE_ITEM);
    VariableItem* data_pin = variable_item_list_get(app->sub_menu, DATA_PIN_ITEM);

    variable_item_set_locked(data_mode, (LWCRunMode)(index) == Demo, GPIO_ONLY);
    variable_item_set_locked(data_pin, (LWCRunMode)(index) == Demo, GPIO_ONLY);
    variable_item_set_item_label(start, run_mode_start_text[index]);
#endif
}

void lwc_data_mode_change_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    ProtoConfig* config = lwc_get_protocol_config(app->state);

    uint8_t index = variable_item_get_current_value_index(item);
    config->data_mode = (LWCDataMode)(index);
    variable_item_set_current_value_text(item, data_mode_names[index]);
}

void lwc_data_pin_change_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    ProtoConfig* config = lwc_get_protocol_config(app->state);

    uint8_t index = variable_item_get_current_value_index(item);
    config->data_pin = (LWCDataPin)(index);
    variable_item_set_current_value_text(item, data_pin_names[index]);
}

void lwc_enter_item_callback(void* context, uint32_t index) {
    App* app = context;

    if(index == START_ITEM) {
        store_proto_config(app->state);
        lwc_app_backlight_on_persist(app);
        scene_manager_next_scene(app->scene_manager, lwc_get_start_scene_for_protocol(app->state));
    }
}

void lwc_sub_menu_scene_on_enter(void* context) {
    App* app = context;

    lwc_app_backlight_on_reset(app);

    ProtoConfig* config = lwc_get_protocol_config(app->state);

#ifdef FW_ORIGIN_Momentum
    variable_item_list_add(app->sub_menu, run_mode_start_text[config->run_mode], 0, NULL, app);
#else
    variable_item_list_add(app->sub_menu, start_mode_text, 0, NULL, app);
#endif

    variable_item_list_set_enter_callback(app->sub_menu, lwc_enter_item_callback, app);

    VariableItem* run_mode = variable_item_list_add(
        app->sub_menu, LWC_UI_TEXT("Run mode", "运行模式"), __lwc_number_of_run_modes, lwc_run_mode_change_callback, app);

    variable_item_set_current_value_index(run_mode, config->run_mode);
    variable_item_set_current_value_text(run_mode, run_mode_names[config->run_mode]);

    VariableItem* data_mode = variable_item_list_add(
        app->sub_menu, LWC_UI_TEXT("GPIO data", "GPIO 数据"), __lwc_number_of_data_modes, lwc_data_mode_change_callback, app);

    variable_item_set_current_value_index(data_mode, config->data_mode);
    variable_item_set_current_value_text(data_mode, data_mode_names[config->data_mode]);

    VariableItem* data_pin = variable_item_list_add(
        app->sub_menu, LWC_UI_TEXT("Data pin", "数据引脚"), __lwc_number_of_data_pins, lwc_data_pin_change_callback, app);

    variable_item_set_current_value_index(data_pin, config->data_pin);
    variable_item_set_current_value_text(data_pin, data_pin_names[config->data_pin]);

#ifdef FW_ORIGIN_Momentum
    variable_item_set_locked(data_mode, config->run_mode == Demo, GPIO_ONLY);
    variable_item_set_locked(data_pin, config->run_mode == Demo, GPIO_ONLY);

    variable_item_list_set_header(app->sub_menu, get_protocol_name(app->state->lwc_type));
#endif

    view_dispatcher_switch_to_view(app->view_dispatcher, LWCSubMenuView);
}

/** main menu event handler - switches scene based on the event */
bool lwc_sub_menu_scene_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void lwc_sub_menu_scene_on_exit(void* context) {
    App* app = context;
    variable_item_list_reset(app->sub_menu);
}
