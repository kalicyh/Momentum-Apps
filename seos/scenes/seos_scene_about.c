#include "../seos_i.h"
#include <dolphin/dolphin.h>

static char* about =
    SEOS_UI_TEXT("This software incorporates a third-party implementation of Seos™ technology. It is not developed, authorized, licensed, or endorsed by HID Global, ASSA ABLOY, or any of their affiliates. References to Seos™ are solely for descriptive and compatibility purposes.\nNo guarantee of compatibility or functionality is made. This implementation may not work with all Seos™-enabled systems, and its performance, security, and reliability are not assured. Users assume all risks associated with its use.\nSeos™, HID Global, and ASSA ABLOY are trademarks or registered trademarks of their respective owners. This software is not associated with or sponsored by them in any way.",
                 "本软件包含 Seos™ 技术的第三方实现。未经 HID Global、ASSA ABLOY 或其关联公司开发、授权、许可或认可。提及 Seos™ 仅用于描述和兼容性目的。\n不保证兼容性或功能性。本实现可能不适用于所有 Seos™ 系统，其性能、安全性和可靠性不作保证。使用者需自行承担所有风险。\nSeos™、HID Global 和 ASSA ABLOY 是其各自所有者的商标或注册商标。本软件与它们无任何关联或赞助关系。");

void seos_scene_about_widget_callback(GuiButtonType result, InputType type, void* context) {
    Seos* seos = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(seos->view_dispatcher, result);
    }
}

void seos_scene_about_on_enter(void* context) {
    Seos* seos = context;

    furi_string_reset(seos->text_box_store);
    FuriString* str = seos->text_box_store;
    furi_string_cat_printf(str, "%s\n", about);

    text_box_set_font(seos->text_box, TextBoxFontText);
    text_box_set_text(seos->text_box, furi_string_get_cstr(seos->text_box_store));
    view_dispatcher_switch_to_view(seos->view_dispatcher, SeosViewTextBox);
}

bool seos_scene_about_on_event(void* context, SceneManagerEvent event) {
    Seos* seos = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(seos->scene_manager);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_previous_scene(seos->scene_manager);
    }
    return consumed;
}

void seos_scene_about_on_exit(void* context) {
    Seos* seos = context;

    // Clear views
    text_box_reset(seos->text_box);
}
