
#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>
#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#include "common/flipboard.h"

#include "app.h"
#include "app_settings.h"
#include "app_config.h"
#include "flipboard_blinky_model.h"

struct AppSettings {
    Flipboard* app;
    FlipboardBlinkyModel* fbm;
    VariableItemList* list;
    TextInput* text_input;
    char buffer[65];
};

static char* display_source_values[] = {
    [FlipboardBlinkySourceAssets] = FLIPBOARD_UI_TEXT("Assets", "素材"),
    [FlipboardBlinkySourceFXBM] = "FXBM",
    [FlipboardBlinkySourceText] = FLIPBOARD_UI_TEXT("TEXT", "文本")};

static char* justification_values[] = {
    [FlipboardBlinkyJustificationLeft] = FLIPBOARD_UI_TEXT("Left", "左对齐"),
    [FlipboardBlinkyJustificationCenter] = FLIPBOARD_UI_TEXT("Center", "居中"),
    [FlipboardBlinkyJustificationRight] = FLIPBOARD_UI_TEXT("Right", "右对齐")};

static void app_settings_populate(AppSettings* settings);
static void app_settings_save(AppSettings* settings);

static void app_settings_display_source_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    AppSettings* settings = (AppSettings*)variable_item_get_context(item);
    flipboard_blinky_set_source_by_index(settings->fbm, index);
    app_settings_populate(settings);
}

static void app_settings_justification_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    AppSettings* settings = (AppSettings*)variable_item_get_context(item);
    flipboard_blinky_set_justification_by_index(settings->fbm, index);
    app_settings_populate(settings);
}

static uint32_t app_settings_show_settings(void* context) {
    UNUSED(context);
    return 1; // Config view
}

static void app_settings_text_entered(void* context) {
    AppSettings* settings = (AppSettings*)context;
    ViewDispatcher* vd = flipboard_get_view_dispatcher(settings->app);
    uint8_t index = settings->buffer[0];
    furi_check(index < 4);
    strncpy(settings->fbm->render_model.line[index], settings->buffer + 1, 64);
    view_dispatcher_switch_to_view(vd, app_settings_show_settings(settings));
}

static void app_settings_item_clicked(void* context, uint32_t index) {
    AppSettings* settings = (AppSettings*)context;
    if(index < 2) {
        return;
    }
    index -= 2;

    char header[32];
    char line_char = (char)('1' + index);
    snprintf(
        header,
        sizeof(header),
        FLIPBOARD_UI_TEXT("Enter Line %c", "输入第%c行"),
        line_char);
    text_input_set_header_text(settings->text_input, header);
    text_input_get_view(settings->text_input);
    settings->buffer[0] = index;
    strncpy(settings->buffer + 1, settings->fbm->render_model.line[index], 64);

    text_input_set_result_callback(
        settings->text_input,
        app_settings_text_entered,
        settings,
        settings->buffer + 1,
        sizeof(settings->buffer) - 1,
        true);
    view_set_previous_callback(
        text_input_get_view(settings->text_input), app_settings_show_settings);
    ViewDispatcher* vd = flipboard_get_view_dispatcher(settings->app);
    view_dispatcher_switch_to_view(vd, 5555);
}

static void app_settings_populate(AppSettings* settings) {
    VariableItem* item;
    uint8_t index;

    variable_item_list_reset(settings->list);

    item = variable_item_list_add(
        settings->list,
        FLIPBOARD_UI_TEXT("Display Source", "显示源"),
        3,
        app_settings_display_source_changed,
        settings);
    index = flipboard_blinky_get_source_index(settings->fbm);
    variable_item_set_current_value_index(item, index);
    variable_item_set_current_value_text(
        item, display_source_values[flipboard_blinky_source_order[index]]);

    item = variable_item_list_add(
        settings->list,
        FLIPBOARD_UI_TEXT("Justification", "对齐方式"),
        3,
        app_settings_justification_changed,
        settings);
    index = flipboard_blinky_get_justification_index(settings->fbm);
    variable_item_set_current_value_index(item, index);
    variable_item_set_current_value_text(
        item, justification_values[flipboard_blinky_justification_order[index]]);

    if(flipboard_blinky_get_source(settings->fbm) == FlipboardBlinkySourceText) {
        variable_item_list_add(
            settings->list, FLIPBOARD_UI_TEXT("Line 1", "第1行"), 0, NULL, NULL);
        variable_item_list_add(
            settings->list, FLIPBOARD_UI_TEXT("Line 2", "第2行"), 0, NULL, NULL);
        variable_item_list_add(
            settings->list, FLIPBOARD_UI_TEXT("Line 3", "第3行"), 0, NULL, NULL);
        variable_item_list_add(
            settings->list, FLIPBOARD_UI_TEXT("Line 4", "第4行"), 0, NULL, NULL);
    }
}

static void app_settings_load(AppSettings* settings) {
    furi_assert(settings->fbm);
    saved_struct_load(
        EXT_PATH("apps_data") "/flipboard/blinky.cfg",
        &settings->fbm->render_model,
        sizeof(FlipboardBlinkyRenderModel),
        FLIPBOARD_BLINKY_CONFIG_MAGIC,
        FLIPBOARD_BLINKY_CONFIG_VERSION);
}

static void app_settings_save(AppSettings* settings) {
    furi_assert(settings->fbm);
    bool saved = saved_struct_save(
        EXT_PATH("apps_data") "/flipboard/blinky.cfg",
        &settings->fbm->render_model,
        sizeof(FlipboardBlinkyRenderModel),
        FLIPBOARD_BLINKY_CONFIG_MAGIC,
        FLIPBOARD_BLINKY_CONFIG_VERSION);
    FURI_LOG_D(TAG, "Saved: %d", saved);
}

AppSettings* app_settings_alloc(Flipboard* app) {
    FlipboardModel* fm = flipboard_get_model(app);
    FlipboardBlinkyModel* fbm = flipboard_model_get_custom_data(fm);
    AppSettings* settings = (AppSettings*)malloc(sizeof(AppSettings));
    settings->app = app;
    settings->fbm = fbm;
    app_settings_load(settings);
    settings->list = variable_item_list_alloc();
    settings->text_input = text_input_alloc();
    variable_item_list_set_enter_callback(settings->list, app_settings_item_clicked, settings);
    app_settings_populate(settings);
    ViewDispatcher* vd = flipboard_get_view_dispatcher(settings->app);
    view_dispatcher_add_view(vd, 5555, text_input_get_view(settings->text_input));

    return settings;
}

void app_settings_free(AppSettings* settings) {
    app_settings_save(settings);
    variable_item_list_free(settings->list);
    ViewDispatcher* vd = flipboard_get_view_dispatcher(settings->app);
    view_dispatcher_remove_view(vd, 5555);
    free(settings);
}

View* app_settings_get_view(AppSettings* settings) {
    return variable_item_list_get_view(settings->list);
}
