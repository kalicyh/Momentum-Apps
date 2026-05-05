#include "../mag_i.h"
#include "../mag_state.h"
#include "../helpers/mag_helpers.h"

#ifndef MAGSPOOF_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define MAGSPOOF_UI_TEXT(en, zh) (zh)
#else
#define MAGSPOOF_UI_TEXT(en, zh) (en)
#endif
#endif

#define TAG "MagSceneEmulateConfig"

enum VarItemListIndex {
    VarItemListIndexPinInput,
    VarItemListIndexPinOutput,
    VarItemListIndexPinEnable,
    VarItemListIndexNRepeats,
    VarItemListIndexRepeatModeOn,
    VarItemListIndexAllowUART,
};

static const char* gpio[] = {
    [MagPinA7] = "2 (A7)",
    [MagPinA6] = "3 (A6)",
    [MagPinA4] = "4 (A4)",
    [MagPinB3] = "5 (B3)",
    [MagPinB2] = "6 (B2)",
    [MagPinC3] = "7 (C3)",
    [MagPinC1] = "15 (C1)",
    [MagPinC0] = "16 (C0)",
};
const uint8_t GPIO_COUNT = COUNT_OF(gpio);
// static const char* uart_pins[] = {[DapUartTypeUSART1] = "13,14", [DapUartTypeLPUART1] = "15,16"};
// static const char* uart_swap[] = {[DapUartTXRXNormal] = "No", [DapUartTXRXSwap] = "Yes"};

#define N_REPEATS_COUNT 10
const char* const n_repeats_text[N_REPEATS_COUNT] = {
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "20",
};
const uint32_t n_repeats_value[N_REPEATS_COUNT] = {
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    20,
};

#define OFF_ON_COUNT 2
const char* const off_on_text[OFF_ON_COUNT] = {
    MAGSPOOF_UI_TEXT("OFF", "关"),
    MAGSPOOF_UI_TEXT("ON", "开"),
};

void mag_scene_settings_var_item_list_callback(void* context, uint32_t index) {
    Mag* mag = context;
    view_dispatcher_send_custom_event(mag->view_dispatcher, index);
}

static void mag_scene_settings_set_gpio(VariableItem* item, MagPin* pin_out) {
    MagPin pin = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, gpio[pin]);
    *pin_out = pin;
}

static void mag_scene_settings_set_gpio_input(VariableItem* item) {
    Mag* mag = variable_item_get_context(item);
    mag_scene_settings_set_gpio(item, &mag->state.pin_input);
}

static void mag_scene_settings_set_gpio_output(VariableItem* item) {
    Mag* mag = variable_item_get_context(item);
    mag_scene_settings_set_gpio(item, &mag->state.pin_output);
}

static void mag_scene_settings_set_gpio_enable(VariableItem* item) {
    Mag* mag = variable_item_get_context(item);
    mag_scene_settings_set_gpio(item, &mag->state.pin_enable);
}

static void mag_scene_settings_set_n_repeats(VariableItem* item) {
    Mag* mag = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, n_repeats_text[index]);
    mag->state.n_repeats = n_repeats_value[index];
}

static void mag_scene_settings_set_bool(VariableItem* item, bool* bool_out) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, off_on_text[index]);
    *bool_out = (bool)index;
}

static void mag_scene_settings_set_repeat_mode(VariableItem* item) {
    Mag* mag = variable_item_get_context(item);
    mag_scene_settings_set_bool(item, &mag->state.repeat_mode);
}

static void mag_scene_settings_set_allow_uart(VariableItem* item) {
    Mag* mag = variable_item_get_context(item);

    // rising change when value index is truth-y, and prior value false
    bool rising = !mag->state.allow_uart && !!variable_item_get_current_value_index(item);
    // trigger dialog only on rising change
    if(rising) {
        scene_manager_set_scene_state(mag->scene_manager, MagSceneSettings, (uint32_t)item);
        view_dispatcher_send_custom_event(mag->view_dispatcher, MagEventConfirmDialog);
    }

    // set value & text based on current varitem index
    mag_scene_settings_set_bool(item, &mag->state.allow_uart);
}

static void mag_pin_variable_item_list_add(
    Mag* mag,
    const char* label,
    MagPin pin,
    VariableItemChangeCallback change_callback) {
    VariableItem* item =
        variable_item_list_add(mag->variable_item_list, label, GPIO_COUNT, change_callback, mag);
    variable_item_set_current_value_index(item, pin);
    variable_item_set_current_value_text(item, gpio[pin]);
}

static void mag_bool_variable_item_list_add(
    Mag* mag,
    const char* label,
    bool value,
    VariableItemChangeCallback change_callback) {
    VariableItem* item =
        variable_item_list_add(mag->variable_item_list, label, OFF_ON_COUNT, change_callback, mag);
    uint32_t value_index = (uint32_t)value;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, off_on_text[value_index]);
}

void mag_scene_settings_on_enter(void* context) {
    Mag* mag = context;
    VariableItemList* var_item_list = mag->variable_item_list;
    VariableItem* item;
    uint32_t value_index;

    // reload state in the event temporary changes have been
    // made on the emulate config screen
    // only changes made in this scene should be saved, and this scene
    // should always represent the saved settings, not the transient ones for
    // a given emulation.
    mag_state_load(&mag->state);

    mag_pin_variable_item_list_add(
        mag,
        MAGSPOOF_UI_TEXT("Input pin:", "输入引脚:"),
        mag->state.pin_input,
        mag_scene_settings_set_gpio_input);
    mag_pin_variable_item_list_add(
        mag,
        MAGSPOOF_UI_TEXT("Output pin:", "输出引脚:"),
        mag->state.pin_output,
        mag_scene_settings_set_gpio_output);
    mag_pin_variable_item_list_add(
        mag,
        MAGSPOOF_UI_TEXT("Enable pin:", "使能引脚:"),
        mag->state.pin_enable,
        mag_scene_settings_set_gpio_enable);

    mag_bool_variable_item_list_add(
        mag,
        MAGSPOOF_UI_TEXT("Repeat default:", "默认重复:"),
        mag->state.repeat_mode,
        mag_scene_settings_set_repeat_mode);

    item = variable_item_list_add(
        var_item_list,
        MAGSPOOF_UI_TEXT("# repeats: ", "重复次数: "),
        N_REPEATS_COUNT,
        mag_scene_settings_set_n_repeats,
        mag);
    value_index = value_index_uint32(mag->state.n_repeats, n_repeats_value, N_REPEATS_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, n_repeats_text[value_index]);

    mag_bool_variable_item_list_add(
        mag,
        MAGSPOOF_UI_TEXT("UART MSR:", "UART 磁卡:"),
        mag->state.allow_uart,
        mag_scene_settings_set_allow_uart);

    variable_item_list_set_enter_callback(
        var_item_list, mag_scene_settings_var_item_list_callback, mag);

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(mag->scene_manager, MagSceneSettings));

    view_dispatcher_switch_to_view(mag->view_dispatcher, MagViewVariableItemList);
}

void mag_scene_settings_dialog_invalid_pins(Mag* mag) {
    SceneManager* scene_manager = mag->scene_manager;

    DialogMessage* message = dialog_message_alloc();

    dialog_message_set_header(
        message,
        MAGSPOOF_UI_TEXT("Invalid Pin Config!", "引脚配置无效!"),
        64,
        0,
        AlignCenter,
        AlignTop);
    dialog_message_set_buttons(
        message, MAGSPOOF_UI_TEXT("Modify", "修改"), NULL, MAGSPOOF_UI_TEXT("Reset", "重置"));
    dialog_message_set_text(
        message,
        MAGSPOOF_UI_TEXT(
            "Pins cannot overlap.\nChange, or reset to defaults.",
            "引脚不能重叠.\n请修改或重置为默认值."),
        64,
        32,
        AlignCenter,
        AlignCenter);
    DialogMessageButton res = dialog_message_show(furi_record_open(RECORD_DIALOGS), message);
    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);
    if(res == DialogMessageButtonRight) {
        mag_state_gpio_reset(&mag->state);
        scene_manager_previous_scene(scene_manager);
    }
}

bool mag_scene_settings_on_event(void* context, SceneManagerEvent event) {
    Mag* mag = context;
    SceneManager* scene_manager = mag->scene_manager;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeBack:
        // when attempting to exit, validate pin configuration
        // if invalid, prompt
        consumed = true;

        if(!mag_state_gpio_is_valid(&mag->state)) {
            mag_scene_settings_dialog_invalid_pins(mag);
        } else {
            scene_manager_previous_scene(scene_manager);
        }

        break;
    case SceneManagerEventTypeCustom:
        // scene_manager_set_scene_state(mag->scene_manager, MagSceneSettings, event.event);
        consumed = true;
        if(event.event == MagEventConfirmDialog) {
            DialogMessage* msg = dialog_message_alloc();
            dialog_message_set_header(
                msg, MAGSPOOF_UI_TEXT("UART MSR", "UART 磁卡读取"), 64, 0, AlignCenter, AlignTop);
            dialog_message_set_buttons(
                msg, MAGSPOOF_UI_TEXT("No", "否"), NULL, MAGSPOOF_UI_TEXT("Yes", "是"));
            dialog_message_set_text(
                msg,
                MAGSPOOF_UI_TEXT(
                    "This option requires a\nUART-compatible mag reader.\nIs it installed?\n",
                    "此选项需要 UART\n兼容的磁卡读取器.\n是否已安装?\n"),
                64,
                32,
                AlignCenter,
                AlignCenter);
            DialogMessageButton res = dialog_message_show(furi_record_open(RECORD_DIALOGS), msg);
            if(res != DialogMessageButtonRight) {
                // if not "Yes", reset to "OFF" (0 / false-y)
                VariableItem* item =
                    (VariableItem*)scene_manager_get_scene_state(scene_manager, MagSceneSettings);
                variable_item_set_current_value_index(item, 0);
                mag_scene_settings_set_bool(item, &mag->state.allow_uart);
            }
            dialog_message_free(msg);
            furi_record_close(RECORD_DIALOGS);
            // clear item from scene state
            scene_manager_set_scene_state(scene_manager, MagSceneSettings, 0);
        }
        break;
    default:
        break;
    }

    return consumed;
}

void mag_scene_settings_on_exit(void* context) {
    Mag* mag = context;

    variable_item_list_reset(mag->variable_item_list);

    mag_state_save(&mag->state);
}
