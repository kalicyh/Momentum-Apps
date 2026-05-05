#include "../weebo_i.h"

#define TAG "ScenesavedMenu"

enum SubmenuIndex {
    SubmenuIndexWrite = 0,
    SubmenuIndexEmulate,
    SubmenuIndexDuplicate,
    SubmenuIndexInfo,
};

void weebo_scene_saved_menu_submenu_callback(void* context, uint32_t index) {
    Weebo* weebo = context;
    view_dispatcher_send_custom_event(weebo->view_dispatcher, index);
}

void weebo_scene_saved_menu_on_enter(void* context) {
    Weebo* weebo = context;
    Submenu* submenu = weebo->submenu;
    submenu_reset(submenu);

    submenu_add_item(
        submenu, WEEBO_UI_TEXT("Write", "写入"), SubmenuIndexWrite, weebo_scene_saved_menu_submenu_callback, weebo);
    submenu_add_item(
        submenu, WEEBO_UI_TEXT("Emulate", "模拟"), SubmenuIndexEmulate, weebo_scene_saved_menu_submenu_callback, weebo);
    submenu_add_item(
        submenu,
        WEEBO_UI_TEXT("Duplicate", "复制"),
        SubmenuIndexDuplicate,
        weebo_scene_saved_menu_submenu_callback,
        weebo);
    submenu_add_item(
        submenu, WEEBO_UI_TEXT("Info", "信息"), SubmenuIndexInfo, weebo_scene_saved_menu_submenu_callback, weebo);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(weebo->scene_manager, WeeboSceneSavedMenu));
    view_dispatcher_switch_to_view(weebo->view_dispatcher, WeeboViewMenu);
}

bool weebo_scene_saved_menu_on_event(void* context, SceneManagerEvent event) {
    Weebo* weebo = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(weebo->scene_manager, WeeboSceneSavedMenu, event.event);
        if(event.event == SubmenuIndexWrite) {
            scene_manager_next_scene(weebo->scene_manager, WeeboSceneWrite);
            consumed = true;
        } else if(event.event == SubmenuIndexEmulate) {
            scene_manager_next_scene(weebo->scene_manager, WeeboSceneEmulate);
            consumed = true;
        } else if(event.event == SubmenuIndexDuplicate) {
            weebo_remix(weebo);
            scene_manager_next_scene(weebo->scene_manager, WeeboSceneSaveName);
            consumed = true;
        } else if(event.event == SubmenuIndexInfo) {
            scene_manager_next_scene(weebo->scene_manager, WeeboSceneInfo);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            weebo->scene_manager, WeeboSceneMainMenu);
    }

    return consumed;
}

void weebo_scene_saved_menu_on_exit(void* context) {
    Weebo* weebo = context;

    submenu_reset(weebo->submenu);
}
