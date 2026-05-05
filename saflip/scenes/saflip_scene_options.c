#include "../saflip.h"
#include <dolphin/dolphin.h>

enum {
    SaflipSceneOptionsMenuItemSave,
    SaflipSceneOptionsMenuItemSaveNFC,
    SaflipSceneOptionsMenuItemEmulate,
    SaflipSceneOptionsMenuItemWrite,
    SaflipSceneOptionsMenuItemEdit,
    SaflipSceneOptionsMenuItemChangeUID,
    SaflipSceneOptionsMenuItemInfo,
};

typedef enum {
    // NfcSceneSaflipStateInMainView = 0 << 0,
    NfcSceneSaflipStateInSubView = 1 << 0,
} SaflipSceneState;

void saflip_scene_options_byte_input_callback(void* context) {
    SaflipApp* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, SaflipViewSubmenu);
}

void saflip_scene_options_menu_callback(void* context, uint32_t index) {
    SaflipApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void saflip_scene_options_on_enter(void* context) {
    SaflipApp* app = context;

    submenu_reset(app->submenu);
    submenu_add_item(
        app->submenu,
        SAFLIP_UI_TEXT("Save", "保存"),
        SaflipSceneOptionsMenuItemSave,
        saflip_scene_options_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        SAFLIP_UI_TEXT("Save as NFC", "保存为NFC"),
        SaflipSceneOptionsMenuItemSaveNFC,
        saflip_scene_options_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        SAFLIP_UI_TEXT("Emulate", "模拟"),
        SaflipSceneOptionsMenuItemEmulate,
        saflip_scene_options_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        SAFLIP_UI_TEXT("Write", "写入"),
        SaflipSceneOptionsMenuItemWrite,
        saflip_scene_options_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        SAFLIP_UI_TEXT("Edit", "编辑"),
        SaflipSceneOptionsMenuItemEdit,
        saflip_scene_options_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        SAFLIP_UI_TEXT("Change UID", "修改UID"),
        SaflipSceneOptionsMenuItemChangeUID,
        saflip_scene_options_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        SAFLIP_UI_TEXT("Info", "信息"),
        SaflipSceneOptionsMenuItemInfo,
        saflip_scene_options_menu_callback,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SaflipViewSubmenu);
}

bool saflip_scene_options_on_event(void* context, SceneManagerEvent event) {
    SaflipApp* app = context;
    bool consumed = false;

    SaflipSceneState state = scene_manager_get_scene_state(app->scene_manager, SaflipSceneOptions);

    if(event.type == SceneManagerEventTypeBack) {
        if(state & NfcSceneSaflipStateInSubView) {
            state &= ~NfcSceneSaflipStateInSubView;
            scene_manager_set_scene_state(app->scene_manager, SaflipSceneOptions, state);
            view_dispatcher_switch_to_view(app->view_dispatcher, SaflipViewSubmenu);
            consumed = true;
        } else {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, SaflipSceneInfo);
        }
    } else if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SaflipSceneOptionsMenuItemSave:
            scene_manager_set_scene_state(
                app->scene_manager, SaflipSceneSave, SaflipSaveModeSaflip);
            scene_manager_next_scene(app->scene_manager, SaflipSceneSave);
            break;

        case SaflipSceneOptionsMenuItemSaveNFC:
            scene_manager_set_scene_state(app->scene_manager, SaflipSceneSave, SaflipSaveModeNFC);
            scene_manager_next_scene(app->scene_manager, SaflipSceneSave);
            break;

        case SaflipSceneOptionsMenuItemEmulate:
            scene_manager_next_scene(app->scene_manager, SaflipSceneEmulate);
            break;

        case SaflipSceneOptionsMenuItemWrite:
            scene_manager_set_scene_state(
                app->scene_manager, SaflipSceneDetectCard, SaflipDetectCardModeWrite);
            scene_manager_next_scene(app->scene_manager, SaflipSceneDetectCard);
            break;

        case SaflipSceneOptionsMenuItemEdit:
            scene_manager_set_scene_state(app->scene_manager, SaflipSceneEdit, 0);

            // If we've previously been to the edit scene, go back to it rather than adding a new item to the history
            if(scene_manager_has_previous_scene(app->scene_manager, SaflipSceneEdit)) {
                scene_manager_search_and_switch_to_previous_scene(
                    app->scene_manager, SaflipSceneEdit);
            } else {
                scene_manager_next_scene(app->scene_manager, SaflipSceneEdit);
            }
            break;

        case SaflipSceneOptionsMenuItemChangeUID:
            byte_input_set_header_text(app->byte_input, SAFLIP_UI_TEXT("Change UID", "修改UID"));
            byte_input_set_result_callback(
                app->byte_input,
                saflip_scene_options_byte_input_callback,
                NULL,
                app,
                app->uid,
                app->uid_len);

            scene_manager_set_scene_state(
                app->scene_manager, SaflipSceneOptions, NfcSceneSaflipStateInSubView);

            view_dispatcher_switch_to_view(app->view_dispatcher, SaflipViewByteInput);

            break;

        case SaflipSceneOptionsMenuItemInfo:
            scene_manager_next_scene(app->scene_manager, SaflipSceneInfo);
            break;
        }
    }

    return consumed;
}

void saflip_scene_options_on_exit(void* context) {
    SaflipApp* app = context;

    UNUSED(app);
}
