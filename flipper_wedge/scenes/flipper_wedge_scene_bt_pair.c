#include "../flipper_wedge.h"
#include <gui/elements.h>

typedef struct {
    Widget* widget;
} BtPairSceneContext;

static void flipper_wedge_scene_bt_pair_rebuild_widget(FlipperWedge* app, Widget* widget) {
    widget_reset(widget);

    // Title
    widget_add_string_element(
        widget,
        64,
        4,
        AlignCenter,
        AlignTop,
        FontPrimary,
        FLIPPER_WEDGE_UI_TEXT("Bluetooth Pairing", "\xe8\x93\x9d\xe7\x89\x99\xe9\x85\x8d\xe5\xaf\xb9"));

    // Instructions
    widget_add_string_element(
        widget,
        4,
        18,
        AlignLeft,
        AlignTop,
        FontSecondary,
        FLIPPER_WEDGE_UI_TEXT("1. Open Bluetooth settings", "1. \xe6\x89\x93\xe5\xbc\x80\xe8\x93\x9d\xe7\x89\x99\xe8\xae\xbe\xe7\xbd\xae"));
    widget_add_string_element(
        widget,
        4,
        28,
        AlignLeft,
        AlignTop,
        FontSecondary,
        FLIPPER_WEDGE_UI_TEXT("   on your device", "   \xe5\x9c\xa8\xe6\x82\xa8\xe7\x9a\x84\xe8\xae\xbe\xe5\xa4\x87\xe4\xb8\x8a"));
    widget_add_string_element(
        widget,
        4,
        38,
        AlignLeft,
        AlignTop,
        FontSecondary,
        FLIPPER_WEDGE_UI_TEXT("2. Select 'HID-[name]'", "2. \xe9\x80\x89\xe6\x8b\xa9 'HID-[name]'"));

    // Connection status
    bool bt_connected = flipper_wedge_hid_is_bt_connected(flipper_wedge_get_hid(app));
    if(bt_connected) {
        widget_add_string_element(
            widget,
            4,
            52,
            AlignLeft,
            AlignTop,
            FontSecondary,
            FLIPPER_WEDGE_UI_TEXT("Status: Connected!", "\xe7\x8a\xb6\xe6\x80\x81: \xe5\xb7\xb2\xe8\xbf\x9e\xe6\x8e\xa5!"));
    } else {
        widget_add_string_element(
            widget,
            4,
            52,
            AlignLeft,
            AlignTop,
            FontSecondary,
            FLIPPER_WEDGE_UI_TEXT("Status: Waiting...", "\xe7\x8a\xb6\xe6\x80\x81: \xe7\xad\x89\xe5\xbe\x85\xe4\xb8\xad..."));
    }
}

void flipper_wedge_scene_bt_pair_on_enter(void* context) {
    FlipperWedge* app = context;

    // Keep display backlight on while pairing
    notification_message(app->notification, &sequence_display_backlight_enforce_on);

    // Allocate scene context
    BtPairSceneContext* scene_ctx = malloc(sizeof(BtPairSceneContext));
    scene_ctx->widget = widget_alloc();

    // Build initial widget content
    flipper_wedge_scene_bt_pair_rebuild_widget(app, scene_ctx->widget);

    // Add view and switch to it
    view_dispatcher_add_view(
        app->view_dispatcher,
        FlipperWedgeViewIdBtPair,
        widget_get_view(scene_ctx->widget));
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipperWedgeViewIdBtPair);

    // Store scene context
    scene_manager_set_scene_state(
        app->scene_manager,
        FlipperWedgeSceneBtPair,
        (uint32_t)scene_ctx);
}

bool flipper_wedge_scene_bt_pair_on_event(void* context, SceneManagerEvent event) {
    FlipperWedge* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        // Update widget on tick to show connection status changes
        BtPairSceneContext* scene_ctx = (BtPairSceneContext*)scene_manager_get_scene_state(
            app->scene_manager,
            FlipperWedgeSceneBtPair);

        if(scene_ctx && scene_ctx->widget) {
            flipper_wedge_scene_bt_pair_rebuild_widget(app, scene_ctx->widget);
        }
        consumed = true;
    }

    return consumed;
}

void flipper_wedge_scene_bt_pair_on_exit(void* context) {
    FlipperWedge* app = context;

    // Retrieve scene context
    BtPairSceneContext* scene_ctx = (BtPairSceneContext*)scene_manager_get_scene_state(
        app->scene_manager,
        FlipperWedgeSceneBtPair);

    if(scene_ctx) {
        if(scene_ctx->widget) {
            view_dispatcher_remove_view(app->view_dispatcher, FlipperWedgeViewIdBtPair);
            widget_free(scene_ctx->widget);
        }
        free(scene_ctx);
    }

    scene_manager_set_scene_state(app->scene_manager, FlipperWedgeSceneBtPair, 0);

    // Return backlight to auto mode
    notification_message(app->notification, &sequence_display_backlight_enforce_auto);
}
