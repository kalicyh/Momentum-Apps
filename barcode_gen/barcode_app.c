#include "barcode_app.h"

#include "barcode_app_icons.h"
#include <assets_icons.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

/**
 * Opens a file browser dialog and returns the filepath of the selected file
 * 
 * @param folder  the folder to view when the browser opens
 * @param file_path a string pointer for the file_path when a file is selected, 
 *                  file_path will be the folder path is nothing is selected
 * @returns true if a file is selected
*/

static NotificationApp* barcode_notifications;

const NotificationSequence sequence_display_backlight_barcode = {
    &message_force_display_brightness_setting_1f,
    &message_display_backlight_on,
    &message_do_not_reset,
    NULL,
};

static bool select_file(const char* folder, FuriString* file_path) {
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, "", &I_barcode_10);
    browser_options.base_path = DEFAULT_USER_BARCODES;
    furi_string_set(file_path, folder);

    bool res = dialog_file_browser_show(dialogs, file_path, file_path, &browser_options);

    furi_record_close(RECORD_DIALOGS);

    return res;
}

/**
 * Reads the data from a file and stores them in the FuriStrings raw_type and raw_data
*/
ErrorCode read_raw_data(FuriString* file_path, FuriString* raw_type, FuriString* raw_data) {
    //Open Storage
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    ErrorCode reason = OKCode;

    if(!flipper_format_file_open_existing(ff, furi_string_get_cstr(file_path))) {
        FURI_LOG_E(TAG, "Could not open file %s", furi_string_get_cstr(file_path));
        reason = FileOpening;
    } else {
        if(!flipper_format_read_string(ff, "Type", raw_type)) {
            FURI_LOG_E(TAG, "Could not read \"Type\" string");
            reason = InvalidFileData;
        }
        if(!flipper_format_read_string(ff, "Data", raw_data)) {
            FURI_LOG_E(TAG, "Could not read \"Data\" string");
            reason = InvalidFileData;
        }
    }

    //Close Storage
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    return reason;
}

/**
 * Gets the file name from a file path
 * @param file_path  the file path
 * @param file_name  the FuriString to store the file name
 * @param remove_extension  true if the extension should be removed, otherwise false
*/
bool get_file_name_from_path(FuriString* file_path, FuriString* file_name, bool remove_extension) {
    if(file_path == NULL || file_name == NULL) {
        return false;
    }
    uint32_t slash_index = furi_string_search_rchar(file_path, '/', 0);
    if(slash_index == FURI_STRING_FAILURE || slash_index >= (furi_string_size(file_path) - 1)) {
        return false;
    }

    furi_string_set(file_name, file_path);
    furi_string_right(file_name, slash_index + 1);
    if(remove_extension) {
        uint32_t ext_index = furi_string_search_rchar(file_name, '.', 0);
        if(ext_index != FURI_STRING_FAILURE && ext_index < (furi_string_size(file_path))) {
            furi_string_left(file_name, ext_index);
        }
    }

    return true;
}

/**
 * Creates the barcode folder
*/
void init_folder() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FURI_LOG_I(TAG, "Creating barcodes folder");
    if(storage_simply_mkdir(storage, DEFAULT_USER_BARCODES)) {
        FURI_LOG_I(TAG, "Barcodes folder successfully created!");
    } else {
        FURI_LOG_I(TAG, "Barcodes folder already exists.");
    }
    furi_record_close(RECORD_STORAGE);
}

void select_barcode_item(BarcodeApp* app) {
    FuriString* file_path = furi_string_alloc();
    FuriString* raw_type = furi_string_alloc();
    FuriString* raw_data = furi_string_alloc();

    //this determines if the data was read correctly or if the
    bool loaded_success = true;
    ErrorCode reason = OKCode;

    bool file_selected = select_file(DEFAULT_USER_BARCODES, file_path);
    if(file_selected) {
        FURI_LOG_I(TAG, "The file selected is %s", furi_string_get_cstr(file_path));
        Barcode* barcode = app->barcode_view;

        reason = read_raw_data(file_path, raw_type, raw_data);
        if(reason != OKCode) {
            loaded_success = false;
            FURI_LOG_E(TAG, "Could not read data correctly");
        }

        //Free the data from the previous barcode
        barcode_free_model(barcode);

        with_view_model(
            barcode->view,
            BarcodeModel * model,
            {
                model->file_path = furi_string_alloc_set(file_path);

                model->data = malloc(sizeof(BarcodeData));
                model->data->valid = loaded_success;

                if(loaded_success) {
                    model->data->raw_data = furi_string_alloc_set(raw_data);
                    model->data->correct_data = furi_string_alloc();

                    model->data->type_obj = get_type(raw_type);

                    barcode_loader(model->data);
                } else {
                    model->data->reason = reason;
                }
            },
            true);

        notification_message(barcode_notifications, &sequence_display_backlight_barcode);

        view_dispatcher_switch_to_view(app->view_dispatcher, BarcodeView);
    }

    furi_string_free(raw_type);
    furi_string_free(raw_data);
    furi_string_free(file_path);
}

void edit_barcode_item(BarcodeApp* app) {
    FuriString* file_path = furi_string_alloc();
    FuriString* file_name = furi_string_alloc();
    FuriString* raw_type = furi_string_alloc();
    FuriString* raw_data = furi_string_alloc();

    //this determines if the data was read correctly or if the
    ErrorCode reason = OKCode;

    bool file_selected = select_file(DEFAULT_USER_BARCODES, file_path);
    if(file_selected) {
        FURI_LOG_I(TAG, "The file selected is %s", furi_string_get_cstr(file_path));
        CreateView* create_view_object = app->create_view;

        reason = read_raw_data(file_path, raw_type, raw_data);
        if(reason != OKCode) {
            FURI_LOG_E(TAG, "Could not read data correctly");
            with_view_model(
                app->message_view->view,
                MessageViewModel * model,
                { model->message = get_error_code_message(reason); },
                true);

            view_dispatcher_switch_to_view(
                create_view_object->barcode_app->view_dispatcher, MessageErrorView);

        } else {
            BarcodeTypeObj* type_obj = get_type(raw_type);
            if(type_obj->type == UNKNOWN) {
                type_obj = barcode_type_objs[0];
            }
            get_file_name_from_path(file_path, file_name, true);

            create_view_free_model(create_view_object);
            with_view_model(
                create_view_object->view,
                CreateViewModel * model,
                {
                    model->selected_menu_item = 0;
                    model->barcode_type = type_obj;
                    model->file_path = furi_string_alloc_set(file_path);
                    model->file_name = furi_string_alloc_set(file_name);
                    model->barcode_data = furi_string_alloc_set(raw_data);
                    model->mode = EditMode;
                },
                true);
            view_dispatcher_switch_to_view(app->view_dispatcher, CreateBarcodeView);
        }
    }

    furi_string_free(raw_type);
    furi_string_free(raw_data);
    furi_string_free(file_name);
    furi_string_free(file_path);
}

void create_barcode_item(BarcodeApp* app) {
    CreateView* create_view_object = app->create_view;

    create_view_free_model(create_view_object);

    with_view_model(
        create_view_object->view,
        CreateViewModel * model,
        {
            model->selected_menu_item = 0;
            model->barcode_type = barcode_type_objs[0];
            model->file_path = furi_string_alloc();
            model->file_name = furi_string_alloc();
            model->barcode_data = furi_string_alloc();
            model->mode = NewMode;
        },
        true);
    view_dispatcher_switch_to_view(app->view_dispatcher, CreateBarcodeView);
}

void submenu_callback(void* context, uint32_t index) {
    furi_assert(context);

    BarcodeApp* app = context;

    if(index == SelectBarcodeItem) {
        select_barcode_item(app);
    } else if(index == EditBarcodeItem) {
        edit_barcode_item(app);
    } else if(index == CreateBarcodeItem) {
        create_barcode_item(app);
    } else if(index == AboutWidgetItem) {
        view_dispatcher_switch_to_view(app->view_dispatcher, AboutWidgetView);
    } else if(index == ErrorCodesWidgetItem) {
        view_dispatcher_switch_to_view(app->view_dispatcher, ErrorCodesWidgetView);
    }
}

uint32_t create_view_callback(void* context) {
    UNUSED(context);
    return CreateBarcodeView;
}

uint32_t main_menu_callback(void* context) {
    UNUSED(context);
    return MainMenuView;
}

uint32_t exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

void free_app(BarcodeApp* app) {
    FURI_LOG_I(TAG, "Freeing Data");

    init_folder();
    free_types();

    view_dispatcher_remove_view(app->view_dispatcher, TextInputView);
    text_input_free(app->text_input);

    view_dispatcher_remove_view(app->view_dispatcher, AboutWidgetView);
    widget_free(app->about_widget);

    view_dispatcher_remove_view(app->view_dispatcher, ErrorCodesWidgetView);
    widget_free(app->error_codes_widget);

    view_dispatcher_remove_view(app->view_dispatcher, MessageErrorView);
    message_view_free(app->message_view);

    view_dispatcher_remove_view(app->view_dispatcher, MainMenuView);
    submenu_free(app->main_menu);

    view_dispatcher_remove_view(app->view_dispatcher, CreateBarcodeView);
    create_view_free(app->create_view);

    view_dispatcher_remove_view(app->view_dispatcher, BarcodeView);
    barcode_free(app->barcode_view);

    //free the dispatcher
    view_dispatcher_free(app->view_dispatcher);

    furi_message_queue_free(app->event_queue);

    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    free(app);
}

/*void set_backlight_brightness(float brightness) {
    NotificationApp* barcode_notifications = furi_record_open(RECORD_NOTIFICATION);
    barcode_notifications->settings.display_brightness = brightness;
    notification_message(barcode_notifications, &sequence_display_backlight_on);
}*/

int32_t barcode_main(void* p) {
    UNUSED(p);
    BarcodeApp* app = malloc(sizeof(BarcodeApp));
    init_types();
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Register view port in GUI
    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->main_menu = submenu_alloc();
    submenu_add_item(app->main_menu, BARCODE_GEN_UI_TEXT("Load Barcode", "加载条码"), SelectBarcodeItem, submenu_callback, app);
    view_set_previous_callback(submenu_get_view(app->main_menu), exit_callback);
    view_dispatcher_add_view(app->view_dispatcher, MainMenuView, submenu_get_view(app->main_menu));

    submenu_add_item(app->main_menu, BARCODE_GEN_UI_TEXT("Edit Barcode", "编辑条码"), EditBarcodeItem, submenu_callback, app);

    barcode_notifications = furi_record_open(RECORD_NOTIFICATION);
    // Save original brightness
    //float originalBrightness = barcode_notifications->settings.display_brightness;
    // force backlight and increase brightness
    //notification_message(barcode_notifications, &sequence_display_backlight_enforce_on);
    //set_backlight_brightness(10); // set to highest

    /*****************************
     * Creating Text Input View
     ******************************/
    app->text_input = text_input_alloc();
    view_set_previous_callback(text_input_get_view(app->text_input), create_view_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, TextInputView, text_input_get_view(app->text_input));

    /*****************************
     * Creating Message View
     ******************************/
    app->message_view = message_view_allocate(app);
    view_dispatcher_add_view(
        app->view_dispatcher, MessageErrorView, message_get_view(app->message_view));

    /*****************************
     * Creating Create View
     ******************************/
    app->create_view = create_view_allocate(app);
    submenu_add_item(app->main_menu, BARCODE_GEN_UI_TEXT("Create Barcode", "创建条码"), CreateBarcodeItem, submenu_callback, app);
    view_set_previous_callback(create_get_view(app->create_view), main_menu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, CreateBarcodeView, create_get_view(app->create_view));

    /*****************************
     * Creating Error Codes View
     ******************************/
    app->error_codes_widget = widget_alloc();
    widget_add_text_scroll_element(
        app->error_codes_widget,
        0,
        0,
        128,
        64,
        BARCODE_GEN_UI_TEXT(
            "\e#Error Codes\n"
            "\e#Wrong # Of Characters\n"
            "The barcode data has too \nmany or too few characters\n"
            "UPC-A: 11-12 characters\n"
            "EAN-8: 7-8 characters\n"
            "EAN-13: 12-13 characters\n"
            "Code128C - even # of \ncharacters\n"
            "\n"
            "\e#Invalid Characters\n"
            "The barcode data has invalid \ncharacters.\n"
            "Ex: UPC-A, EAN-8, EAN-13 barcodes can only have \nnumbers while Code128 can \nhave almost any character\n"
            "\n"
            "\e#Unsupported Type\n"
            "The barcode type is not \nsupported by this application\n"
            "\n"
            "\e#File Opening Error\n"
            "The barcode file could not be opened. One reason could be \nthat the file no longer exists\n"
            "\n"
            "\e#Invalid File Data\n"
            "The barcode file could not find the keys \"Type\" or \"Data\". \nThis usually occurs when you edit the file manually and \naccidently change the keys\n"
            "\n"
            "\e#Missing Encoding Table\n"
            "The encoding table files are \nmissing. This only occurs \nwhen you need to handle the \nencoding files manually. If you \ndownload the files from the \napp store this should not \noccur\n"
            "\n"
            "\e#Encoding Table Error\n"
            "This occurs when the \nprogram cannot find a \ncharacter in the encoding \ntable, meaning that either the\ncharacter isn't supported \nor the character is missing \nfrom the encoding table\n",
            "\e#错误代码\n"
            "\e#字符数量错误\n"
            "条码数据的字符数量\n过多或过少\n"
            "UPC-A: 11-12个字符\n"
            "EAN-8: 7-8个字符\n"
            "EAN-13: 12-13个字符\n"
            "Code128C - 偶数个字符\n"
            "\n"
            "\e#无效字符\n"
            "条码数据包含无效字符\n"
            "例如: UPC-A, EAN-8, EAN-13\n条码只能包含数字，而\nCode128可以包含几乎所有\n字符\n"
            "\n"
            "\e#不支持的类型\n"
            "此应用程序不支持\n该条码类型\n"
            "\n"
            "\e#文件打开错误\n"
            "无法打开条码文件。\n可能的原因是文件\n已不存在\n"
            "\n"
            "\e#无效的文件数据\n"
            "条码文件中找不到\n\"Type\"或\"Data\"键。\n这通常发生在手动编辑\n文件时意外更改了键名\n"
            "\n"
            "\e#缺少编码表\n"
            "编码表文件缺失。\n仅在需要手动处理\n编码文件时才会出现。\n从应用商店下载的\n文件通常不会出现\n此问题\n"
            "\n"
            "\e#编码表错误\n"
            "当程序在编码表中\n找不到某个字符时\n会出现此错误，\n意味着该字符不受支持\n或编码表中缺少\n该字符\n"));
    view_set_previous_callback(widget_get_view(app->error_codes_widget), main_menu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, ErrorCodesWidgetView, widget_get_view(app->error_codes_widget));
    submenu_add_item(
        app->main_menu, BARCODE_GEN_UI_TEXT("Error Codes Info", "错误代码信息"), ErrorCodesWidgetItem, submenu_callback, app);

    /*****************************
     * Creating About View
     ******************************/
    app->about_widget = widget_alloc();
    widget_add_text_scroll_element(
        app->about_widget,
        0,
        0,
        128,
        64,
        BARCODE_GEN_UI_TEXT(
            "This is a barcode generator\n"
            "capable of generating UPC-A,\n"
            "EAN-8, EAN-13, Code-39,\n"
            "Codabar, and Code-128\n"
            "\n"
            "author: @Kingal1337\n"
            "\n"
            "For more information or\n"
            "issues, go to\n"
            "https://github.com/Kingal1337/flipper-barcode-generator",
            "这是一个条码生成器\n"
            "支持生成 UPC-A、\n"
            "EAN-8、EAN-13、Code-39、\n"
            "Codabar 和 Code-128\n"
            "\n"
            "作者: @Kingal1337\n"
            "\n"
            "如需更多信息或\n"
            "报告问题，请访问\n"
            "https://github.com/Kingal1337/flipper-barcode-generator"));
    view_set_previous_callback(widget_get_view(app->about_widget), main_menu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, AboutWidgetView, widget_get_view(app->about_widget));
    submenu_add_item(app->main_menu, BARCODE_GEN_UI_TEXT("About", "关于"), AboutWidgetItem, submenu_callback, app);

    /*****************************
     * Creating Barcode View
     ******************************/
    app->barcode_view = barcode_view_allocate(app);
    view_set_previous_callback(barcode_get_view(app->barcode_view), main_menu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, BarcodeView, barcode_get_view(app->barcode_view));

    //switch view to submenu and run dispatcher
    view_dispatcher_switch_to_view(app->view_dispatcher, MainMenuView);
    view_dispatcher_run(app->view_dispatcher);

    free_app(app);
    notification_message_block(barcode_notifications, &sequence_display_backlight_enforce_auto);
    //set_backlight_brightness(originalBrightness);
    furi_record_close(RECORD_NOTIFICATION);

    return 0;
}
