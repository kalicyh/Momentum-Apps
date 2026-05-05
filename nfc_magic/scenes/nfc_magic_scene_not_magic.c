#include "../nfc_magic_app_i.h"

void nfc_magic_scene_not_magic_widget_callback(GuiButtonType result, InputType type, void* context) {
    NfcMagicApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_magic_scene_not_magic_on_enter(void* context) {
    NfcMagicApp* instance = context;
    Widget* widget = instance->widget;

    notification_message(instance->notifications, &sequence_error);

    widget_add_string_element(
        widget, 3, 4, AlignLeft, AlignTop, FontPrimary, NFC_MAGIC_UI_TEXT("Incorrect card type", "卡片类型错误"));
    widget_add_string_multiline_element(
        widget,
        4,
        17,
        AlignLeft,
        AlignTop,
        FontSecondary,
        NFC_MAGIC_UI_TEXT("Not magic or unsupported\ncard. Only Gen1, Gen2 and \nGen4 UMC cards supported.", "非魔术卡或不支持\n的卡片. 仅支持\nGen1, Gen2, Gen4 UMC."));
    widget_add_button_element(
        widget, GuiButtonTypeLeft, NFC_MAGIC_UI_TEXT("Retry", "重试"), nfc_magic_scene_not_magic_widget_callback, instance);

    // Setup and start worker
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewWidget);
}

bool nfc_magic_scene_not_magic_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(instance->scene_manager);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, NfcMagicSceneStart);
    }
    return consumed;
}

void nfc_magic_scene_not_magic_on_exit(void* context) {
    NfcMagicApp* instance = context;

    widget_reset(instance->widget);
}
