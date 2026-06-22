#include "../picopass_i.h"

void picopass_scene_clean_card_failure_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    furi_assert(context);
    Picopass* picopass = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(picopass->view_dispatcher, result);
    }
}

void picopass_scene_clean_card_failure_on_enter(void* context) {
    Picopass* picopass = context;
    Widget* widget = picopass->widget;

    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        PICOPASS_UI_TEXT("Retry", "重试"),
        picopass_scene_clean_card_failure_widget_callback,
        picopass);

    widget_add_button_element(
        widget,
        GuiButtonTypeRight,
        PICOPASS_UI_TEXT("Menu", "菜单"),
        picopass_scene_clean_card_failure_widget_callback,
        picopass);

    widget_add_string_element(
        widget, 64, 5, AlignCenter, AlignCenter, FontSecondary, PICOPASS_UI_TEXT("Clean Failed!", "清理失败!"));

    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewWidget);
}

bool picopass_scene_clean_card_failure_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(picopass->scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                picopass->scene_manager, PicopassSceneStart);
        }
    }
    return consumed;
}

void picopass_scene_clean_card_failure_on_exit(void* context) {
    Picopass* picopass = context;
    widget_reset(picopass->widget);
}
