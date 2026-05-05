#include "../metroflip_i.h"
#include <nfc/protocols/mf_classic/mf_classic.h>

void metroflip_scene_start_submenu_callback(void* context, uint32_t index) {
    Metroflip* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void metroflip_scene_start_on_enter(void* context) {
    Metroflip* app = context;
    Submenu* submenu = app->submenu;

    // Clean up any previously loaded MFC data when returning to start
    if(app->mfc_data) {
        mf_classic_free(app->mfc_data);
        app->mfc_data = NULL;
    }

    submenu_set_header(submenu, "Metroflip");

    submenu_add_item(
        submenu, METROFLIP_UI_TEXT("Scan Card", "\xe6\x89\xab\xe6\x8f\x8f\xe5\x8d\xa1\xe7\x89\x87"), MetroflipSceneAuto, metroflip_scene_start_submenu_callback, app);

    submenu_add_item(
        submenu,
        "OV-Chipkaart (unstable)",
        MetroflipSceneOVC,
        metroflip_scene_start_submenu_callback,
        app);

    submenu_add_item(
        submenu, METROFLIP_UI_TEXT("Saved", "\xe5\xb7\xb2\xe4\xbf\x9d\xe5\xad\x98"), MetroflipSceneLoad, metroflip_scene_start_submenu_callback, app);

    submenu_add_item(
        submenu,
        METROFLIP_UI_TEXT("Supported Cards", "\xe6\x94\xaf\xe6\x8c\x81\xe7\x9a\x84\xe5\x8d\xa1\xe7\x89\x87"),
        MetroflipSceneSupported,
        metroflip_scene_start_submenu_callback,
        app);

    submenu_add_item(
        submenu, METROFLIP_UI_TEXT("About", "\xe5\x85\xb3\xe4\xba\x8e"), MetroflipSceneAbout, metroflip_scene_start_submenu_callback, app);

    submenu_add_item(
        submenu, METROFLIP_UI_TEXT("Credits", "\xe8\x87\xb4\xe8\xb0\xa2"), MetroflipSceneCredits, metroflip_scene_start_submenu_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, MetroflipSceneStart));

    notification_message(app->notifications, &sequence_display_backlight_on);
    view_dispatcher_switch_to_view(app->view_dispatcher, MetroflipViewSubmenu);
}

bool metroflip_scene_start_on_event(void* context, SceneManagerEvent event) {
    Metroflip* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, MetroflipSceneStart, event.event);
        consumed = true;
        scene_manager_next_scene(app->scene_manager, event.event);
    }

    return consumed;
}

void metroflip_scene_start_on_exit(void* context) {
    Metroflip* app = context;
    submenu_reset(app->submenu);
}
