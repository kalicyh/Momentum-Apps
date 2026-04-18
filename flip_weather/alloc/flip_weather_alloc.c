#include <alloc/flip_weather_alloc.h>

// Function to allocate resources for the FlipWeatherApp
FlipWeatherApp *flip_weather_app_alloc()
{
    FlipWeatherApp *app = (FlipWeatherApp *)malloc(sizeof(FlipWeatherApp));

    Gui *gui = furi_record_open(RECORD_GUI);

    // initialize uart
    if (!flipper_http_init(flipper_http_rx_callback, app))
    {
        FURI_LOG_E(TAG, "Failed to initialize flipper http");
        return NULL;
    }

    // Allocate the text input buffer
    app->uart_text_input_buffer_size_ssid = 64;
    app->uart_text_input_buffer_size_password = 64;
    if (!easy_flipper_set_buffer(&app->uart_text_input_buffer_ssid, app->uart_text_input_buffer_size_ssid))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->uart_text_input_temp_buffer_ssid, app->uart_text_input_buffer_size_ssid))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->uart_text_input_buffer_password, app->uart_text_input_buffer_size_password))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->uart_text_input_temp_buffer_password, app->uart_text_input_buffer_size_password))
    {
        return NULL;
    }

    // Allocate ViewDispatcher
    if (!easy_flipper_set_view_dispatcher(&app->view_dispatcher, gui, app))
    {
        return NULL;
    }
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, flip_weather_custom_event_callback);
    // Main view
    if (!easy_flipper_set_view(&app->view_loader, FlipWeatherViewLoader, flip_weather_loader_draw_callback, NULL, callback_to_submenu, &app->view_dispatcher, app))
    {
        return NULL;
    }
    flip_weather_loader_init(app->view_loader);

    // Widget
    if (!easy_flipper_set_widget(
            &app->widget,
            FlipWeatherViewAbout,
            FLIP_WEATHER_UI_TEXT(
                "FlipWeather v1.3\n-----\nUse WiFi to get GPS and \nWeather information.\n-----\nwww.github.com/jblanked",
                "FlipWeather v1.3\n-----\n使用 WiFi 获取 GPS\n和天气信息。\n-----\nwww.github.com/jblanked"),
            callback_to_submenu,
            &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_widget(
            &app->widget_result,
            FlipWeatherViewWidgetResult,
            FLIP_WEATHER_UI_TEXT("Error, try again.", "出错了，请重试。"),
            callback_to_submenu,
            &app->view_dispatcher))
    {
        return NULL;
    }

    // Text Input
    if (!easy_flipper_set_uart_text_input(
            &app->uart_text_input_ssid,
            FlipWeatherViewTextInputSSID,
            FLIP_WEATHER_UI_TEXT("Enter SSID", "输入 SSID"),
            app->uart_text_input_temp_buffer_ssid,
            app->uart_text_input_buffer_size_ssid,
            text_updated_ssid,
            callback_to_wifi_settings,
            &app->view_dispatcher,
            app))
    {
        return NULL;
    }
    if (!easy_flipper_set_uart_text_input(
            &app->uart_text_input_password,
            FlipWeatherViewTextInputPassword,
            FLIP_WEATHER_UI_TEXT("Enter Password", "输入密码"),
            app->uart_text_input_temp_buffer_password,
            app->uart_text_input_buffer_size_password,
            text_updated_password,
            callback_to_wifi_settings,
            &app->view_dispatcher,
            app))
    {
        return NULL;
    }

    // Variable Item List
    if (!easy_flipper_set_variable_item_list(&app->variable_item_list, FlipWeatherViewSettings, settings_item_selected, callback_to_submenu, &app->view_dispatcher, app))
    {
        return NULL;
    }
    app->variable_item_ssid = variable_item_list_add(
        app->variable_item_list, FLIP_WEATHER_UI_TEXT("SSID", "SSID"), 0, NULL, NULL);
    app->variable_item_password = variable_item_list_add(
        app->variable_item_list, FLIP_WEATHER_UI_TEXT("Password", "密码"), 0, NULL, NULL);
    app->variable_item_temperature_unit = variable_item_list_add(
        app->variable_item_list,
        FLIP_WEATHER_UI_TEXT("Temperature", "温度"),
        2,
        temperature_unit_change,
        app);
    variable_item_set_current_value_text(app->variable_item_ssid, "");
    variable_item_set_current_value_text(app->variable_item_password, "");
    variable_item_set_current_value_index(app->variable_item_temperature_unit, 0);
    variable_item_set_current_value_text(
        app->variable_item_temperature_unit, FLIP_WEATHER_UI_TEXT("Celsius", "摄氏"));

    // Submenu
    if (!easy_flipper_set_submenu(
            &app->submenu,
            FlipWeatherViewSubmenu,
            FLIP_WEATHER_UI_TEXT("FlipWeather v1.3", "FlipWeather v1.3"),
            callback_exit_app,
            &app->view_dispatcher))
    {
        return NULL;
    }
    submenu_add_item(
        app->submenu,
        FLIP_WEATHER_UI_TEXT("Weather", "天气"),
        FlipWeatherSubmenuIndexWeather,
        callback_submenu_choices,
        app);
    submenu_add_item(
        app->submenu,
        FLIP_WEATHER_UI_TEXT("GPS", "GPS"),
        FlipWeatherSubmenuIndexGPS,
        callback_submenu_choices,
        app);
    submenu_add_item(
        app->submenu,
        FLIP_WEATHER_UI_TEXT("About", "关于"),
        FlipWeatherSubmenuIndexAbout,
        callback_submenu_choices,
        app);
    submenu_add_item(
        app->submenu,
        FLIP_WEATHER_UI_TEXT("Settings", "设置"),
        FlipWeatherSubmenuIndexSettings,
        callback_submenu_choices,
        app);

    // load settings
    if (load_settings(app->uart_text_input_buffer_ssid, app->uart_text_input_buffer_size_ssid, app->uart_text_input_buffer_password, app->uart_text_input_buffer_size_password, &use_fahrenheit))
    {
        // Update variable items
        if (app->variable_item_ssid)
            variable_item_set_current_value_text(app->variable_item_ssid, app->uart_text_input_buffer_ssid);
        // dont show password

        // Copy items into their temp buffers with safety checks
        if (app->uart_text_input_buffer_ssid && app->uart_text_input_temp_buffer_ssid)
        {
            strncpy(app->uart_text_input_temp_buffer_ssid, app->uart_text_input_buffer_ssid, app->uart_text_input_buffer_size_ssid - 1);
            app->uart_text_input_temp_buffer_ssid[app->uart_text_input_buffer_size_ssid - 1] = '\0';
        }
        if (app->uart_text_input_buffer_password && app->uart_text_input_temp_buffer_password)
        {
            strncpy(app->uart_text_input_temp_buffer_password, app->uart_text_input_buffer_password, app->uart_text_input_buffer_size_password - 1);
            app->uart_text_input_temp_buffer_password[app->uart_text_input_buffer_size_password - 1] = '\0';
        }

        // Apply loaded temperature unit
        if (app->variable_item_temperature_unit)
        {
            variable_item_set_current_value_index(app->variable_item_temperature_unit, use_fahrenheit ? 1 : 0);
            variable_item_set_current_value_text(
                app->variable_item_temperature_unit,
                use_fahrenheit ? FLIP_WEATHER_UI_TEXT("Fahrenheit", "华氏") :
                                 FLIP_WEATHER_UI_TEXT("Celsius", "摄氏"));
        }
    }

    // Switch to the main view
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipWeatherViewSubmenu);

    return app;
}
