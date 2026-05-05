#include "../xremote.h"

void xremote_scene_wip_on_enter(void* context) {
    furi_assert(context);
    XRemote* app = context;
    Popup* popup = app->popup;

    //popup_set_icon(popup, 0, 2, &I_DolphinMafia_115x62);
    popup_set_header(popup, XREMOTE_UI_TEXT("SubGhz coming soon", "SubGhz \xe5\x8d\xb3\xe5\xb0\x86\xe4\xb8\x8a\xe7\xba\xbf"), 10, 19, AlignLeft, AlignBottom);
    popup_set_text(popup, XREMOTE_UI_TEXT("Check back later", "\xe8\xaf\xb7\xe7\xa8\x8d\xe5\x90\x8e\xe6\xa3\x80\xe6\x9f\xa5"), 10, 29, AlignLeft, AlignBottom);
    popup_set_text(popup, XREMOTE_UI_TEXT("Press back long", "\xe9\x95\xbf\xe6\x8c\x89\xe8\xbf\x94\xe5\x9b\x9e"), 10, 39, AlignLeft, AlignBottom);

    popup_set_callback(popup, xremote_popup_closed_callback);
    popup_set_context(popup, context);
    popup_set_timeout(popup, 100);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, XRemoteViewIdWip);
}

bool xremote_scene_wip_on_event(void* context, SceneManagerEvent event) {
    XRemote* app = context;
    UNUSED(app);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == XRemoteCustomEventTypePopupClosed) {
        }
        consumed = true;
    }

    return consumed;
}

void xremote_scene_wip_on_exit(void* context) {
    XRemote* app = context;
    UNUSED(app);
}