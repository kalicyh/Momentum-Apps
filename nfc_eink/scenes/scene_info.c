#include "../nfc_eink_app_i.h"

void nfc_eink_scene_info_on_enter(void* context) {
    NfcEinkApp* instance = context;

    if(!instance->screen_loaded) {
        const NfcEinkScreenInfo* info = instance->info_temp;
        instance->screen = nfc_eink_screen_alloc(info->screen_manufacturer);
        nfc_eink_screen_init(instance->screen, info->name);

        if(!nfc_eink_screen_load_data(
               furi_string_get_cstr(instance->file_path), instance->screen, info)) {
            nfc_eink_screen_free(instance->screen);
            instance->screen_loaded = false;
        } else {
            instance->screen_loaded = true;
        }
    }
    const NfcEinkScreenInfo* info = nfc_eink_screen_get_image_info(instance->screen);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(temp_str, "\e#%s\n", NFC_EINK_UI_TEXT("Information", "详细信息"));
    furi_string_cat_printf(
        temp_str, "%s %s\n", NFC_EINK_UI_TEXT("Name:", "名称:"), info->name);
    furi_string_cat_printf(
        temp_str,
        "%s %s\n",
        NFC_EINK_UI_TEXT("Vendor:", "厂商:"),
        nfc_eink_screen_get_manufacturer_name(info->screen_manufacturer));
    furi_string_cat_printf(
        temp_str, "%s %d x %d\n", NFC_EINK_UI_TEXT("Resolution:", "分辨率:"), info->width, info->height);

    furi_string_cat_printf(
        temp_str,
        "%s %d bytes\n",
        NFC_EINK_UI_TEXT("Size:", "大小:"),
        nfc_eink_screen_get_image_size(instance->screen));

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcEinkViewWidget);
}

bool nfc_eink_scene_info_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_eink_scene_info_on_exit(void* context) {
    NfcEinkApp* instance = context;
    widget_reset(instance->widget);

    if(!scene_manager_has_previous_scene(instance->scene_manager, NfcEinkAppSceneEmulate) &&
       instance->screen_loaded) {
        nfc_eink_screen_free(instance->screen);
        instance->screen_loaded = false;
    }
}
