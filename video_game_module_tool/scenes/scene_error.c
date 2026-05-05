#include "app_i.h"

#include <furi.h>
#include <notification/notification_messages.h>

#include "custom_event.h"
#include "video_game_module_tool_icons.h"

static void
    scene_error_button_callback(GuiButtonType button_type, InputType input_type, void* context) {
    App* app = context;
    if(input_type == InputTypeShort && button_type == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, CustomEventRetryRequested);
    }
}

void scene_error_on_enter(void* context) {
    App* app = context;

    widget_add_icon_element(app->widget, 83, 22, &I_WarningDolphinFlip_45x42);
    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, VGM_UI_TEXT("Retry", "重试"), scene_error_button_callback, app);
    widget_add_string_element(
        app->widget, 64, 0, AlignCenter, AlignTop, FontPrimary, VGM_UI_TEXT("Installation Failed!", "安装失败!"));

    const char* error_msg;
    if(app->flasher_error == FlasherErrorBadFile) {
        error_msg = VGM_UI_TEXT("This file is\ncorrupted or\nunsupported", "文件已损坏\n或不支持");
    } else if(app->flasher_error == FlasherErrorDisconnect) {
        error_msg = VGM_UI_TEXT("The module was\ndisconnected\nduring the update", "更新过程中\n模块已断开");
    } else if(app->flasher_error == FlasherErrorUnknown) {
        error_msg = VGM_UI_TEXT("An unknown error\nhas occurred", "发生未知错误");
    } else {
        furi_crash();
    }

    widget_add_string_multiline_element(
        app->widget, 0, 28, AlignLeft, AlignCenter, FontSecondary, error_msg);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIdWidget);

    notification_message(app->notification, &sequence_error);
    notification_message(app->notification, &sequence_set_red_255);
}

bool scene_error_on_event(void* context, SceneManagerEvent event) {
    App* app = context;

    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == CustomEventRetryRequested) {
            scene_manager_search_and_switch_to_previous_scene(app->scene_manager, SceneProbe);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        furi_string_reset(app->file_path);
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, SceneProbe);
        consumed = true;
    }

    return consumed;
}

void scene_error_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
    notification_message(app->notification, &sequence_reset_red);
}
