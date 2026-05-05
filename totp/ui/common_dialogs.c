#include "common_dialogs.h"
#include "constants.h"

static DialogMessageButton totp_dialogs_common(PluginState* plugin_state, const char* text) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_buttons(message, TOTP_UI_TEXT("Exit", "退出"), NULL, NULL);
    dialog_message_set_text(
        message, text, SCREEN_WIDTH_CENTER, SCREEN_HEIGHT_CENTER, AlignCenter, AlignCenter);
    DialogMessageButton result = dialog_message_show(plugin_state->dialogs_app, message);
    dialog_message_free(message);
    return result;
}

DialogMessageButton totp_dialogs_config_loading_error(PluginState* plugin_state) {
    return totp_dialogs_common(plugin_state, TOTP_UI_TEXT("An error has occurred\nduring loading config file", "加载配置文件时\n发生错误"));
}

DialogMessageButton totp_dialogs_config_updating_error(PluginState* plugin_state) {
    return totp_dialogs_common(plugin_state, TOTP_UI_TEXT("An error has occurred\nduring updating config file", "更新配置文件时\n发生错误"));
}