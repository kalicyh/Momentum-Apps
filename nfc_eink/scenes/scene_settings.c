#include "../nfc_eink_app_i.h"

static const char* nfc_eink_scene_settings_write_mode[] = {
    NFC_EINK_UI_TEXT("Strict", "严格"),
    NFC_EINK_UI_TEXT("Size", "尺寸"),
    NFC_EINK_UI_TEXT("Vendor", "厂商"),
    NFC_EINK_UI_TEXT("Free", "自由"),
};

static const char* nfc_eink_scene_settings_invert_image[] = {
    NFC_EINK_UI_TEXT("OFF", "关"),
    NFC_EINK_UI_TEXT("ON", "开"),
};

static void nfc_eink_scene_settings_write_mode_change_callback(VariableItem* item) {
    NfcEinkApp* instance = variable_item_get_context(item);
    const uint8_t index = variable_item_get_current_value_index(item);

    instance->settings.write_mode = (NfcEinkWriteMode)index;

    variable_item_set_current_value_text(item, nfc_eink_scene_settings_write_mode[index]);
}

static void nfc_eink_scene_settings_invert_image_change_callback(VariableItem* item) {
    NfcEinkApp* instance = variable_item_get_context(item);
    const uint8_t index = variable_item_get_current_value_index(item);

    instance->settings.invert_image = (index == 1);

    variable_item_set_current_value_text(item, nfc_eink_scene_settings_invert_image[index]);
}

void nfc_eink_scene_settings_on_enter(void* context) {
    NfcEinkApp* instance = context;
    VariableItemList* var_item_list = instance->var_item_list;

    VariableItem* item;
    uint8_t value_index = 0;

    item = variable_item_list_add(
        var_item_list,
        NFC_EINK_UI_TEXT("Write mode", "写入模式"),
        COUNT_OF(nfc_eink_scene_settings_write_mode),
        nfc_eink_scene_settings_write_mode_change_callback,
        instance);

    value_index = instance->settings.write_mode;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, nfc_eink_scene_settings_write_mode[value_index]);

    item = variable_item_list_add(
        var_item_list,
        NFC_EINK_UI_TEXT("Invert image", "反色图像"),
        COUNT_OF(nfc_eink_scene_settings_invert_image),
        nfc_eink_scene_settings_invert_image_change_callback,
        instance);
    value_index = instance->settings.invert_image ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, nfc_eink_scene_settings_invert_image[value_index]);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcEinkViewVarItemList);
}

bool nfc_eink_scene_settings_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);

    return false;
}

void nfc_eink_scene_settings_on_exit(void* context) {
    NfcEinkApp* instance = context;

    variable_item_list_reset(instance->var_item_list);
    nfc_eink_save_settings(instance);
}
