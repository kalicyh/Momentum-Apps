#include "../nfc_magic_app_i.h"

void nfc_magic_scene_gen4_fail_widget_callback(GuiButtonType result, InputType type, void* context) {
    NfcMagicApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_magic_scene_gen4_fail_on_enter(void* context) {
    NfcMagicApp* instance = context;
    Widget* widget = instance->widget;

    notification_message(instance->notifications, &sequence_error);

    widget_add_icon_element(widget, 83, 22, &I_WarningDolphinFlip_45x42);
    widget_add_string_element(
        widget,
        7,
        4,
        AlignLeft,
        AlignTop,
        FontPrimary,
        NFC_MAGIC_UI_TEXT("Something gone wrong!", "发生错误!"));
    widget_add_string_multiline_element(
        widget,
        7,
        17,
        AlignLeft,
        AlignTop,
        FontSecondary,
        NFC_MAGIC_UI_TEXT("No response. Maybe\nnot Gen4 card?", "无响应，可能\n不是 Gen4 卡?"));

    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        NFC_MAGIC_UI_TEXT("Back", "返回"),
        nfc_magic_scene_gen4_fail_widget_callback,
        instance);

    // Setup and start worker
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewWidget);
}

bool nfc_magic_scene_gen4_fail_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcMagicSceneStart);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, NfcMagicSceneStart);
    }
    return consumed;
}

void nfc_magic_scene_gen4_fail_on_exit(void* context) {
    NfcMagicApp* instance = context;

    widget_reset(instance->widget);
}
