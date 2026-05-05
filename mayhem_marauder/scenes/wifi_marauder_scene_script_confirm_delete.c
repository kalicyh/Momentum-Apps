#include "../wifi_marauder_app_i.h"

void wifi_marauder_scene_script_confirm_delete_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    WifiMarauderApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void wifi_marauder_scene_script_confirm_delete_on_enter(void* context) {
    WifiMarauderApp* app = context;

    widget_add_button_element(
        app->widget,
        GuiButtonTypeLeft,
        WIFI_MARAUDER_UI_TEXT("No", "否"),
        wifi_marauder_scene_script_confirm_delete_widget_callback,
        app);
    widget_add_button_element(
        app->widget,
        GuiButtonTypeRight,
        WIFI_MARAUDER_UI_TEXT("Yes", "是"),
        wifi_marauder_scene_script_confirm_delete_widget_callback,
        app);

    widget_add_string_element(
        app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, WIFI_MARAUDER_UI_TEXT("Are you sure?", "确定吗?"));
    widget_add_text_box_element(
        app->widget,
        0,
        12,
        128,
        38,
        AlignCenter,
        AlignCenter,
        WIFI_MARAUDER_UI_TEXT("The script will be\npermanently deleted", "脚本将被\n永久删除"),
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewWidget);
}

bool wifi_marauder_scene_script_confirm_delete_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        // get which button press: "Yes" or "No"
        if(event.event == GuiButtonTypeRight) {
            // Yes
            if(app->script != NULL) {
                char script_path[256];
                snprintf(
                    script_path,
                    sizeof(script_path),
                    "%s/%s.json",
                    MARAUDER_APP_FOLDER_SCRIPTS,
                    app->script->name);
                storage_simply_remove(app->storage, script_path);
                wifi_marauder_script_free(app->script);
                app->script = NULL;

                DialogMessage* message = dialog_message_alloc();
                dialog_message_set_text(message, WIFI_MARAUDER_UI_TEXT("Deleted!", "已删除!"), 83, 19, AlignLeft, AlignBottom);
                dialog_message_set_icon(message, &I_DolphinMafia_119x62, 0, 2);
                dialog_message_set_buttons(message, NULL, WIFI_MARAUDER_UI_TEXT("Ok", "确定"), NULL);
                dialog_message_show(app->dialogs, message);
                dialog_message_free(message);
            }
        }
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void wifi_marauder_scene_script_confirm_delete_on_exit(void* context) {
    WifiMarauderApp* app = context;
    widget_reset(app->widget);
}
