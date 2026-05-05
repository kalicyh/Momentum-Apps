#include "../helpers/subghz/subghz.h"
#include "../xremote.h"

enum SubmenuIndex {
    SubmenuIndexCreate = 10,
    SubmenuIndexLoad,
    SubmenuIndexEdit,
    SubmenuIndexSettings,
    SubmenuIndexInfoscreen,
};

void xremote_scene_menu_submenu_callback(void* context, uint32_t index) {
    XRemote* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void xremote_scene_menu_on_enter(void* context) {
    XRemote* app = context;

    submenu_add_item(
        app->submenu,
        XREMOTE_UI_TEXT("New Command Chain", "\xe6\x96\xb0\xe5\xbb\xba\xe5\x91\xbd\xe4\xbb\xa4\xe9\x93\xbe"),
        SubmenuIndexCreate,
        xremote_scene_menu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        XREMOTE_UI_TEXT("Run Saved Command", "\xe8\xbf\x90\xe8\xa1\x8c\xe5\xb7\xb2\xe5\xad\x98\xe5\x91\xbd\xe4\xbb\xa4"),
        SubmenuIndexLoad,
        xremote_scene_menu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        XREMOTE_UI_TEXT("Edit / Delete Command", "\xe7\xbc\x96\xe8\xbe\x91/\xe5\x88\xa0\xe9\x99\xa4\xe5\x91\xbd\xe4\xbb\xa4"),
        SubmenuIndexEdit,
        xremote_scene_menu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu, XREMOTE_UI_TEXT("Settings", "\xe8\xae\xbe\xe7\xbd\xae"), SubmenuIndexSettings, xremote_scene_menu_submenu_callback, app);
    submenu_add_item(
        app->submenu, XREMOTE_UI_TEXT("App Info", "\xe5\xba\x94\xe7\x94\xa8\xe4\xbf\xa1\xe6\x81\xaf"), SubmenuIndexInfoscreen, xremote_scene_menu_submenu_callback, app);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, XRemoteSceneMenu));

    view_dispatcher_switch_to_view(app->view_dispatcher, XRemoteViewIdMenu);
}

bool xremote_scene_menu_on_event(void* context, SceneManagerEvent event) {
    XRemote* app = context;
    if(event.type == SceneManagerEventTypeBack) {
        //exit app
        scene_manager_stop(app->scene_manager);
        view_dispatcher_stop(app->view_dispatcher);
        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexCreate) {
            scene_manager_set_scene_state(
                app->scene_manager, XRemoteSceneMenu, SubmenuIndexCreate);
            scene_manager_next_scene(app->scene_manager, XRemoteSceneCreate);
            return true;
        } else if(event.event == SubmenuIndexLoad) {
            scene_manager_set_scene_state(app->scene_manager, XRemoteSceneMenu, SubmenuIndexLoad);
            scene_manager_next_scene(app->scene_manager, XRemoteSceneXrList);
            return true;
        } else if(event.event == SubmenuIndexSettings) {
            scene_manager_set_scene_state(
                app->scene_manager, XRemoteSceneMenu, SubmenuIndexSettings);
            scene_manager_next_scene(app->scene_manager, XRemoteSceneSettings);
            return true;
        } else if(event.event == SubmenuIndexInfoscreen) {
            scene_manager_set_scene_state(
                app->scene_manager, XRemoteSceneMenu, SubmenuIndexInfoscreen);
            scene_manager_next_scene(app->scene_manager, XRemoteSceneInfoscreen);
            return true;
        } else if(event.event == SubmenuIndexEdit) {
            scene_manager_set_scene_state(app->scene_manager, XRemoteSceneMenu, SubmenuIndexEdit);
            scene_manager_next_scene(app->scene_manager, XRemoteSceneXrListEdit);
        }
    }
    return false;
}

void xremote_scene_menu_on_exit(void* context) {
    XRemote* app = context;
    submenu_reset(app->submenu);
}