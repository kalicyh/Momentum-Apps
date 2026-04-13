#include "flipper.h"
#include "app_state.h"
#include "scenes.h"
#include "scene_main_menu.h"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define RESISTORS_UI_TEXT(en, zh) (zh)
#else
#define RESISTORS_UI_TEXT(en, zh) (en)
#endif

/* main menu scene */

/** main menu callback - sends custom events to the scene manager based on the selection */
void resistors_menu_callback(void* context, uint32_t index) {
    App* app = context;
    switch(index) {
    case ResistorsMainMenuSelectionR3:
        scene_manager_handle_custom_event(
            app->scene_manager, ResistorsMainMenuSceneSelectionEventR3);
        break;
    case ResistorsMainMenuSelectionR4:
        scene_manager_handle_custom_event(
            app->scene_manager, ResistorsMainMenuSceneSelectionEventR4);
        break;
    case ResistorsMainMenuSelectionR5:
        scene_manager_handle_custom_event(
            app->scene_manager, ResistorsMainMenuSceneSelectionEventR5);
        break;
    case ResistorsMainMenuSelectionR6:
        scene_manager_handle_custom_event(
            app->scene_manager, ResistorsMainMenuSceneSelectionEventR6);
        break;
    }
}

/** main menu scene - resets the submenu, and gives it content, callbacks and selection enums */
void resistors_main_menu_scene_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, RESISTORS_UI_TEXT("Resistors", "电阻计算"));
    submenu_add_item(
        app->submenu,
        RESISTORS_UI_TEXT("3-bar resistor", "3 环电阻"),
        ResistorsMainMenuSelectionR3,
        resistors_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        RESISTORS_UI_TEXT("4-bar resistor", "4 环电阻"),
        ResistorsMainMenuSelectionR4,
        resistors_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        RESISTORS_UI_TEXT("5-bar resistor", "5 环电阻"),
        ResistorsMainMenuSelectionR5,
        resistors_menu_callback,
        app);
    submenu_add_item(
        app->submenu,
        RESISTORS_UI_TEXT("6-bar resistor", "6 环电阻"),
        ResistorsMainMenuSelectionR6,
        resistors_menu_callback,
        app);
    view_dispatcher_switch_to_view(app->view_dispatcher, ResistorsSubmenuView);
}

/** main menu event handler - switches scene based on the event */
bool resistors_main_menu_scene_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case ResistorsMainMenuSceneSelectionEventR3:
            app_init_resistor(app, R3);
            scene_manager_next_scene(app->scene_manager, ResistorsEditScene);
            consumed = true;
            break;
        case ResistorsMainMenuSceneSelectionEventR4:
            app_init_resistor(app, R4);
            scene_manager_next_scene(app->scene_manager, ResistorsEditScene);
            consumed = true;
            break;
        case ResistorsMainMenuSceneSelectionEventR5:
            app_init_resistor(app, R5);
            scene_manager_next_scene(app->scene_manager, ResistorsEditScene);
            consumed = true;
            break;
        case ResistorsMainMenuSceneSelectionEventR6:
            app_init_resistor(app, R6);
            scene_manager_next_scene(app->scene_manager, ResistorsEditScene);
            consumed = true;
            break;
        }
        break;
    default: // eg. SceneManagerEventTypeBack, SceneManagerEventTypeTick
        consumed = false;
        break;
    }
    return consumed;
}

void resistors_main_menu_scene_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}
