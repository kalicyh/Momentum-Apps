#include "alloc/flip_library_alloc.h"

// Function to allocate resources for the FlipLibraryApp
FlipLibraryApp *flip_library_app_alloc()
{
    FlipLibraryApp *app = (FlipLibraryApp *)malloc(sizeof(FlipLibraryApp));

    Gui *gui = furi_record_open(RECORD_GUI);

    if (!flipper_http_init(flipper_http_rx_callback, app))
    {
        FURI_LOG_E(TAG, "Failed to initialize flipper http");
        return NULL;
    }

    // Allocate the text input buffer
    app->uart_text_input_buffer_size_ssid = 64;
    app->uart_text_input_buffer_size_password = 64;
    app->uart_text_input_buffer_size_query = 64;
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
    if (!easy_flipper_set_buffer(&app->uart_text_input_buffer_query, app->uart_text_input_buffer_size_query))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->uart_text_input_temp_buffer_query, app->uart_text_input_buffer_size_query))
    {
        return NULL;
    }

    // Allocate ViewDispatcher
    if (!easy_flipper_set_view_dispatcher(&app->view_dispatcher, gui, app))
    {
        return NULL;
    }
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, flip_library_custom_event_callback);

    // Main view
    if (!easy_flipper_set_view(&app->view_loader, FlipLibraryViewLoader, flip_library_loader_draw_callback, NULL, callback_to_random_facts, &app->view_dispatcher, app))
    {
        return NULL;
    }
    flip_library_loader_init(app->view_loader);

    // Widget
    if (!easy_flipper_set_widget(&app->widget_about, FlipLibraryViewAbout, FLIP_LIBRARY_UI_TEXT("FlipLibrary v1.5\n-----\nUtilize WiFi to retrieve data\nfrom 20 different APIs.\n-----\nCreated by JBlanked and\nDerek Jamison.\n-----\nwww.github.com/jblanked/\nFlipLibrary\n-----\nPress BACK to return.", "FlipLibrary v1.5\n-----\n使用 WiFi 从\n20 个不同 API 获取数据。\n-----\n由 JBlanked 与\nDerek Jamison 创建。\n-----\nwww.github.com/jblanked/\nFlipLibrary\n-----\n按返回键返回。"), callback_to_submenu, &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_widget(&app->widget_result, FlipLibraryViewWidgetResult, FLIP_LIBRARY_UI_TEXT("Error, try again.", "出错了，请重试。"), callback_to_random_facts, &app->view_dispatcher))
    {
        return NULL;
    }

    // Text Input
    if (!easy_flipper_set_uart_text_input(&app->uart_text_input_ssid, FlipLibraryViewTextInputSSID, FLIP_LIBRARY_UI_TEXT("Enter SSID", "输入 SSID"), app->uart_text_input_temp_buffer_ssid, app->uart_text_input_buffer_size_ssid, text_updated_ssid, callback_to_wifi_settings, &app->view_dispatcher, app))
    {
        return NULL;
    }
    if (!easy_flipper_set_uart_text_input(&app->uart_text_input_password, FlipLibraryViewTextInputPassword, FLIP_LIBRARY_UI_TEXT("Enter Password", "输入密码"), app->uart_text_input_temp_buffer_password, app->uart_text_input_buffer_size_password, text_updated_password, callback_to_wifi_settings, &app->view_dispatcher, app))
    {
        return NULL;
    }
    if (!easy_flipper_set_uart_text_input(&app->uart_text_input_query, FlipLibraryViewTextInputQuery, FLIP_LIBRARY_UI_TEXT("Enter Query", "输入查询"), app->uart_text_input_temp_buffer_query, app->uart_text_input_buffer_size_query, text_updated_query, callback_to_submenu_library, &app->view_dispatcher, app))
    {
        return NULL;
    }

    // Variable Item List
    if (!easy_flipper_set_variable_item_list(&app->variable_item_list_wifi, FlipLibraryViewSettings, settings_item_selected, callback_to_submenu, &app->view_dispatcher, app))
    {
        return NULL;
    }

    app->variable_item_ssid = variable_item_list_add(app->variable_item_list_wifi, "SSID", 0, NULL, NULL);
    app->variable_item_password = variable_item_list_add(app->variable_item_list_wifi, FLIP_LIBRARY_UI_TEXT("Password", "密码"), 0, NULL, NULL);
    app->variable_item_temperature_unit = variable_item_list_add(app->variable_item_list_wifi, FLIP_LIBRARY_UI_TEXT("Weather Unit", "天气单位"), 2, temperature_unit_change, app);
    variable_item_set_current_value_text(app->variable_item_ssid, "");
    variable_item_set_current_value_text(app->variable_item_password, "");
    variable_item_set_current_value_index(app->variable_item_temperature_unit, 0);
    variable_item_set_current_value_text(app->variable_item_temperature_unit, FLIP_LIBRARY_UI_TEXT("Celsius", "摄氏"));

    // Submenu
    if (!easy_flipper_set_submenu(&app->submenu_main, FlipLibraryViewSubmenuMain, "FlipLibrary v1.5", callback_exit_app, &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_submenu(&app->submenu_library, FlipLibraryViewSubmenuLibrary, FLIP_LIBRARY_UI_TEXT("Library", "资料库"), callback_to_submenu, &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_submenu(&app->submenu_random_facts, FlipLibraryViewRandomFacts, FLIP_LIBRARY_UI_TEXT("Random", "随机"), callback_to_submenu_library, &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_submenu(&app->submenu_predict, FlipLibraryViewPredict, FLIP_LIBRARY_UI_TEXT("Predict", "预测"), callback_to_submenu_library, &app->view_dispatcher))
    {
        return NULL;
    }

    submenu_add_item(app->submenu_main, FLIP_LIBRARY_UI_TEXT("Library", "资料库"), FlipLibrarySubmenuIndexLibrary, callback_submenu_choices, app);
    submenu_add_item(app->submenu_main, FLIP_LIBRARY_UI_TEXT("About", "关于"), FlipLibrarySubmenuIndexAbout, callback_submenu_choices, app);
    submenu_add_item(app->submenu_main, FLIP_LIBRARY_UI_TEXT("Settings", "设置"), FlipLibrarySubmenuIndexSettings, callback_submenu_choices, app);
    submenu_add_item(app->submenu_library, "Wikipedia", FlipLibrarySubmenuIndexWiki, callback_submenu_choices, app);
    submenu_add_item(app->submenu_library, FLIP_LIBRARY_UI_TEXT("Dictionary", "词典"), FlipLibrarySubmenuIndexDictionary, callback_submenu_choices, app);
    submenu_add_item(app->submenu_library, FLIP_LIBRARY_UI_TEXT("Predict", "预测"), FlipLibrarySubmenuIndexPredict, callback_submenu_choices, app);
    submenu_add_item(app->submenu_library, FLIP_LIBRARY_UI_TEXT("Random", "随机"), FlipLibrarySubmenuIndexRandomFacts, callback_submenu_choices, app);
    submenu_add_item(app->submenu_library, FLIP_LIBRARY_UI_TEXT("Weather", "天气"), FlipLibrarySubmenuIndexWeather, callback_submenu_choices, app);
    submenu_add_item(app->submenu_library, "GPS", FlipLibrarySubmenuIndexGPS, callback_submenu_choices, app);
    submenu_add_item(app->submenu_library, FLIP_LIBRARY_UI_TEXT("Elevation", "海拔"), FlipLibrarySubmenuIndexElevation, callback_submenu_choices, app);
    submenu_add_item(app->submenu_library, FLIP_LIBRARY_UI_TEXT("Asset Price", "资产价格"), FlipLibrarySubmenuIndexAssetPrice, callback_submenu_choices, app);
    submenu_add_item(app->submenu_library, FLIP_LIBRARY_UI_TEXT("Next Holiday", "下个假日"), FlipLibrarySubmenuIndexNextHoliday, callback_submenu_choices, app);
    //
    submenu_add_item(app->submenu_random_facts, FLIP_LIBRARY_UI_TEXT("Trivia", "冷知识"), FlipLibrarySubmenuIndexRandomTrivia, callback_submenu_choices, app);
    submenu_add_item(app->submenu_random_facts, FLIP_LIBRARY_UI_TEXT("Advice", "建议"), FlipLibrarySubmenuIndexRandomAdvice, callback_submenu_choices, app);
    submenu_add_item(app->submenu_random_facts, FLIP_LIBRARY_UI_TEXT("Quote", "语录"), FlipLibrarySubmenuIndexRandomFactsQuotes, callback_submenu_choices, app);
    submenu_add_item(app->submenu_random_facts, FLIP_LIBRARY_UI_TEXT("Fact", "事实"), FlipLibrarySubmenuIndexRandomFactsAll, callback_submenu_choices, app);
    submenu_add_item(app->submenu_random_facts, FLIP_LIBRARY_UI_TEXT("Cat Fact", "猫咪事实"), FlipLibrarySubmenuIndexRandomFactsCats, callback_submenu_choices, app);
    submenu_add_item(app->submenu_random_facts, FLIP_LIBRARY_UI_TEXT("Dog Fact", "狗狗事实"), FlipLibrarySubmenuIndexRandomFactsDogs, callback_submenu_choices, app);
    submenu_add_item(app->submenu_random_facts, FLIP_LIBRARY_UI_TEXT("Tech Phrase", "技术短语"), FlipLibrarySubmenuIndexRandomTechPhrase, callback_submenu_choices, app);
    submenu_add_item(app->submenu_random_facts, "UUID", FlipLibrarySubmenuIndexRandomUUID, callback_submenu_choices, app);
    submenu_add_item(app->submenu_random_facts, FLIP_LIBRARY_UI_TEXT("Address", "地址"), FlipLibrarySubmenuIndexRandomAddress, callback_submenu_choices, app);
    submenu_add_item(app->submenu_random_facts, FLIP_LIBRARY_UI_TEXT("Credit Card", "信用卡"), FlipLibrarySubmenuIndexRandomCreditCard, callback_submenu_choices, app);
    submenu_add_item(app->submenu_random_facts, FLIP_LIBRARY_UI_TEXT("User Info", "用户信息"), FlipLibrarySubmenuIndexRandomUserInfo, callback_submenu_choices, app);
    //
    submenu_add_item(app->submenu_predict, FLIP_LIBRARY_UI_TEXT("Age", "年龄"), FlipLibrarySubmenuIndexPredictAge, callback_submenu_choices, app);
    submenu_add_item(app->submenu_predict, FLIP_LIBRARY_UI_TEXT("Ethnicity", "族裔"), FlipLibrarySubmenuIndexPredictEthnicity, callback_submenu_choices, app);
    submenu_add_item(app->submenu_predict, FLIP_LIBRARY_UI_TEXT("Gender", "性别"), FlipLibrarySubmenuIndexPredictGender, callback_submenu_choices, app);

    // load settings
    if (load_settings(app->uart_text_input_buffer_ssid, app->uart_text_input_buffer_size_ssid, app->uart_text_input_buffer_password, app->uart_text_input_buffer_size_password, &use_fahrenheit))
    {
        // Update variable items
        if (app->variable_item_ssid)
        {
            variable_item_set_current_value_text(app->variable_item_ssid, app->uart_text_input_buffer_ssid);
        }
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
            variable_item_set_current_value_text(app->variable_item_temperature_unit, use_fahrenheit ? FLIP_LIBRARY_UI_TEXT("Fahrenheit", "华氏") : FLIP_LIBRARY_UI_TEXT("Celsius", "摄氏"));
        }
    }

    // assign app instance
    app_instance = app;

    // start with the main view
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipLibraryViewSubmenuMain);

    return app;
}
