#include "../saflip.h"
#include "furi_hal_rtc.h"

void saflip_scene_variable_keys_dialog_callback(DialogExResult result, void* context) {
    SaflipApp* app = context;

    if(result == DialogExResultLeft) {
        // Return to submenu without modifying anything
        view_dispatcher_switch_to_view(app->view_dispatcher, SaflipViewSubmenu);
    } else {
        // Remove selected item
        uint32_t idx = submenu_get_selected_item(app->submenu);
        for(size_t i = idx + 1; i < app->variable_keys; i++) {
            app->keys[i - 1] = app->keys[i];
        }
        app->variable_keys--;

        // Redraw and return to submenu
        saflip_scene_variable_keys_on_enter(app);
    }
}

void saflip_scene_variable_keys_submenu_callback(void* context, InputType type, uint32_t index) {
    SaflipApp* app = context;

    scene_manager_set_scene_state(app->scene_manager, SaflipSceneVariableKeys, index);

    if(type == InputTypeShort) {
        if(index == app->variable_keys) {
            // Create variable key
            VariableKey key = {.lock_id = 0};
            furi_hal_rtc_get_datetime(&key.creation);
            key.creation.second = 0; // Seconds aren't stored or used
            app->keys[app->variable_keys++] = key;

            scene_manager_set_scene_state(app->scene_manager, SaflipSceneVariableKeyInfo, index);
            scene_manager_next_scene(app->scene_manager, SaflipSceneVariableKeyInfo);
        } else if(index == app->variable_keys + 1) {
            // Toggle optional function
            app->variable_keys_optional_function =
                (app->variable_keys_optional_function + 1) % VariableKeysOptionalFunctionNum;

            // Redraw and return to submenu
            saflip_scene_variable_keys_on_enter(app);
        } else {
            // Edit existing variable key
            scene_manager_set_scene_state(app->scene_manager, SaflipSceneVariableKeyInfo, index);
            scene_manager_next_scene(app->scene_manager, SaflipSceneVariableKeyInfo);
        }
    } else if(type == InputTypeLong && index < app->variable_keys) {
        // Prompt to remove variable key
        dialog_ex_set_header(app->dialog, SAFLIP_UI_TEXT("Remove variable key?", "删除可变密钥?"), 64, 12, AlignCenter, AlignTop);
        dialog_ex_set_left_button_text(app->dialog, SAFLIP_UI_TEXT("Cancel", "取消"));
        dialog_ex_set_right_button_text(app->dialog, SAFLIP_UI_TEXT("Remove", "删除"));
        dialog_ex_set_result_callback(app->dialog, saflip_scene_variable_keys_dialog_callback);
        dialog_ex_set_context(app->dialog, app);
        view_dispatcher_switch_to_view(app->view_dispatcher, SaflipViewDialog);
    }

    UNUSED(index);
}

void saflip_scene_variable_keys_on_enter(void* context) {
    SaflipApp* app = context;

    FuriString* label = furi_string_alloc();

    submenu_reset(app->submenu);

    for(size_t i = 0; i < app->variable_keys; i++) {
        furi_string_printf(
            label,
            "%d @ %04d-%02d-%02d %02d:%02d",
            app->keys[i].lock_id,
            app->keys[i].creation.year,
            app->keys[i].creation.month,
            app->keys[i].creation.day,
            app->keys[i].creation.hour,
            app->keys[i].creation.minute);
        submenu_add_item_ex(
            app->submenu,
            furi_string_get_cstr(label),
            i,
            saflip_scene_variable_keys_submenu_callback,
            app);
    }

    submenu_add_item_ex(
        app->submenu,
        SAFLIP_UI_TEXT("Add variable key...", "添加可变密钥..."),
        app->variable_keys,
        saflip_scene_variable_keys_submenu_callback,
        app);

    switch(app->variable_keys_optional_function) {
    case VariableKeysOptionalFunctionNone:
        furi_string_printf(label, SAFLIP_UI_TEXT("Optional Function: None", "可选功能: 无"));
        break;
    case VariableKeysOptionalFunctionLevelInhibit:
        furi_string_printf(label, SAFLIP_UI_TEXT("Opt. Func: Level Inhibit", "可选功能: 级别禁止"));
        break;
    case VariableKeysOptionalFunctionElectLockUnlock:
        furi_string_printf(label, SAFLIP_UI_TEXT("Opt. Func: Elec. Un/Lock", "可选功能: 电子锁/解锁"));
        break;
    case VariableKeysOptionalFunctionLatchUnlatch:
        furi_string_printf(label, SAFLIP_UI_TEXT("Opt. Func: Latch/UnLatch", "可选功能: 锁舌/解锁"));
        break;
    default:
        furi_string_printf(label, SAFLIP_UI_TEXT("Opt. Func: Unknown", "可选功能: 未知"));
    }
    submenu_add_item_ex(
        app->submenu,
        furi_string_get_cstr(label),
        app->variable_keys + 1,
        saflip_scene_variable_keys_submenu_callback,
        app);

    furi_string_free(label);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, SaflipSceneVariableKeys));

    view_dispatcher_switch_to_view(app->view_dispatcher, SaflipViewSubmenu);
}

bool saflip_scene_variable_keys_on_event(void* context, SceneManagerEvent event) {
    SaflipApp* app = context;
    bool consumed = false;

    UNUSED(event);
    UNUSED(app);

    return consumed;
}

void saflip_scene_variable_keys_on_exit(void* context) {
    SaflipApp* app = context;

    UNUSED(app);
}
