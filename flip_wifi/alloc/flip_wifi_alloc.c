#include <callback/callback.h>

// Function to allocate resources for the FlipWiFiApp
FlipWiFiApp *flip_wifi_app_alloc()
{
    FlipWiFiApp *app = (FlipWiFiApp *)malloc(sizeof(FlipWiFiApp));

    Gui *gui = furi_record_open(RECORD_GUI);

    // Allocate ViewDispatcher
    if (!easy_flipper_set_view_dispatcher(&app->view_dispatcher, gui, app))
    {
        return NULL;
    }

    // Submenu
    if (!easy_flipper_set_submenu(&app->submenu_main, FlipWiFiViewSubmenuMain, VERSION_TAG, callback_exit_app, &app->view_dispatcher))
    {
        return NULL;
    }
    submenu_add_item(
        app->submenu_main,
        FLIP_WIFI_UI_TEXT("Scan", "扫描"),
        FlipWiFiSubmenuIndexWiFiScan,
        callback_submenu_choices,
        app);
    submenu_add_item(
        app->submenu_main,
        FLIP_WIFI_UI_TEXT("Deauthentication", "去认证"),
        FlipWiFiSubmenuIndexWiFiDeauth,
        callback_submenu_choices,
        app);
    submenu_add_item(
        app->submenu_main,
        FLIP_WIFI_UI_TEXT("Captive Portal", "门户页面"),
        FlipWiFiSubmenuIndexWiFiAP,
        callback_submenu_choices,
        app);
    submenu_add_item(
        app->submenu_main,
        FLIP_WIFI_UI_TEXT("Saved APs", "已保存 AP"),
        FlipWiFiSubmenuIndexWiFiSaved,
        callback_submenu_choices,
        app);
    submenu_add_item(
        app->submenu_main,
        FLIP_WIFI_UI_TEXT("Commands", "命令"),
        FlipWiFiSubmenuIndexCommands,
        callback_submenu_choices,
        app);
    submenu_add_item(
        app->submenu_main,
        FLIP_WIFI_UI_TEXT("Info", "信息"),
        FlipWiFiSubmenuIndexAbout,
        callback_submenu_choices,
        app);

    app->fhttp = NULL;

    // Switch to the main view
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipWiFiViewSubmenuMain);

    return app;
}
