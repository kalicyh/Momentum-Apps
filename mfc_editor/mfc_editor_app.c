#include "mfc_editor_app_i.h"

const char* access_data_block_labels[8] = {
    // C3, C2, C1
    MFC_EDITOR_UI_TEXT("Key A: Read, Write, Inc, Dec\nKey B: Read, Write, Inc, Dec", "密钥A: 读/写/增/减\n密钥B: 读/写/增/减"), // 000
    MFC_EDITOR_UI_TEXT("Key A: Read\nKey B: Read, Write", "密钥A: 读\n密钥B: 读/写"), // 001
    MFC_EDITOR_UI_TEXT("Key A: Read\nKey B: Read", "密钥A: 读\n密钥B: 读"), // 010
    MFC_EDITOR_UI_TEXT("Key A: Read, Dec\nKey B: Read, Write, Inc, Dec", "密钥A: 读/减\n密钥B: 读/写/增/减"), // 011
    MFC_EDITOR_UI_TEXT("Key A: Read, Dec\nKey B: Read, Dec", "密钥A: 读/减\n密钥B: 读/减"), // 100
    MFC_EDITOR_UI_TEXT("Key A: No Access\nKey B: Read", "密钥A: 无权限\n密钥B: 读"), // 101
    MFC_EDITOR_UI_TEXT("Key A: No Access\nKey B: Read, Write", "密钥A: 无权限\n密钥B: 读/写"), // 110
    MFC_EDITOR_UI_TEXT("Key A: No Access\nKey B: No Access", "密钥A: 无权限\n密钥B: 无权限"), // 111
};

const char* access_sector_trailer_labels[8] = {
    // C3, C2, C1
    MFC_EDITOR_UI_TEXT("Key A: KA-W, AB-R, KB-RW\nKey B: No Access", "密钥A: KA可写, AB可读, KB可读写\n密钥B: 无权限"), // 000
    MFC_EDITOR_UI_TEXT("Key A: AB-R\nKey B: KA+KB-W, AB-R", "密钥A: AB可读\n密钥B: KA+KB可写, AB可读"), // 001
    MFC_EDITOR_UI_TEXT("Key A: AB+KB-R\nKey B: No Access", "密钥A: AB+KB可读\n密钥B: 无权限"), // 010
    MFC_EDITOR_UI_TEXT("Key A: AB-R\nKey B: AB-R", "密钥A: AB可读\n密钥B: AB可读"), // 011
    MFC_EDITOR_UI_TEXT("Key A: KA-W, AB+KB-RW\nKey B: No Access", "密钥A: KA可写, AB+KB可读写\n密钥B: 无权限"), // 100
    MFC_EDITOR_UI_TEXT("Key A: AB-R\nKey B: AB-RW", "密钥A: AB可读\n密钥B: AB可读写"), // 101
    MFC_EDITOR_UI_TEXT("Key A: AB-R\nKey B: KA+KB-W, AB-RW", "密钥A: AB可读\n密钥B: KA+KB可写, AB可读写"), // 110
    MFC_EDITOR_UI_TEXT("Key A: AB-R\nKey B: AB-R", "密钥A: AB可读\n密钥B: AB可读"), // 111
};

bool mfc_editor_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    MfcEditorApp* instance = context;
    return scene_manager_handle_custom_event(instance->scene_manager, event);
}

bool mfc_editor_app_back_event_callback(void* context) {
    furi_assert(context);
    MfcEditorApp* instance = context;
    return scene_manager_handle_back_event(instance->scene_manager);
}

void mfc_editor_app_tick_event_callback(void* context) {
    furi_assert(context);
    MfcEditorApp* instance = context;
    scene_manager_handle_tick_event(instance->scene_manager);
}

MfcEditorApp* mfc_editor_app_alloc() {
    MfcEditorApp* instance = malloc(sizeof(MfcEditorApp));

    instance->view_dispatcher = view_dispatcher_alloc();
    instance->scene_manager = scene_manager_alloc(&mfc_editor_scene_handlers, instance);
    view_dispatcher_set_event_callback_context(instance->view_dispatcher, instance);
    view_dispatcher_set_custom_event_callback(
        instance->view_dispatcher, mfc_editor_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        instance->view_dispatcher, mfc_editor_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        instance->view_dispatcher, mfc_editor_app_tick_event_callback, 100);

    instance->gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(
        instance->view_dispatcher, instance->gui, ViewDispatcherTypeFullscreen);

    instance->storage = furi_record_open(RECORD_STORAGE);

    instance->dialogs = furi_record_open(RECORD_DIALOGS);

    instance->mf_classic_data = mf_classic_alloc();
    instance->file_path = furi_string_alloc_set(NFC_APP_FOLDER);

    instance->data_view_header = furi_string_alloc();
    instance->data_view_text = furi_string_alloc();

    instance->submenu = submenu_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, MfcEditorAppViewSubmenu, submenu_get_view(instance->submenu));

    instance->popup = popup_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, MfcEditorAppViewPopup, popup_get_view(instance->popup));

    instance->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher,
        MfcEditorAppViewDialogEx,
        dialog_ex_get_view(instance->dialog_ex));

    instance->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher,
        MfcEditorAppViewByteInput,
        byte_input_get_view(instance->byte_input));

    return instance;
}

void mfc_editor_app_free(MfcEditorApp* instance) {
    furi_assert(instance);

    view_dispatcher_remove_view(instance->view_dispatcher, MfcEditorAppViewSubmenu);
    submenu_free(instance->submenu);

    view_dispatcher_remove_view(instance->view_dispatcher, MfcEditorAppViewPopup);
    popup_free(instance->popup);

    view_dispatcher_remove_view(instance->view_dispatcher, MfcEditorAppViewDialogEx);
    dialog_ex_free(instance->dialog_ex);

    view_dispatcher_remove_view(instance->view_dispatcher, MfcEditorAppViewByteInput);
    byte_input_free(instance->byte_input);

    view_dispatcher_free(instance->view_dispatcher);
    scene_manager_free(instance->scene_manager);

    furi_record_close(RECORD_GUI);
    instance->gui = NULL;

    furi_record_close(RECORD_STORAGE);
    instance->storage = NULL;

    furi_record_close(RECORD_DIALOGS);
    instance->dialogs = NULL;

    mf_classic_free(instance->mf_classic_data);
    furi_string_free(instance->file_path);

    furi_string_free(instance->data_view_header);
    furi_string_free(instance->data_view_text);

    free(instance);
}

MfcEditorPromptResponse mfc_editor_load_file(MfcEditorApp* instance, FuriString* file_path) {
    furi_assert(instance);
    furi_assert(file_path);

    MfcEditorPromptResponse result = MfcEditorPromptResponseSuccess;

    NfcDevice* nfc_device = nfc_device_alloc();

    if(!nfc_device_load(nfc_device, furi_string_get_cstr(file_path))) {
        result = MfcEditorPromptResponseFailure;
        dialog_message_show_storage_error(instance->dialogs, MFC_EDITOR_UI_TEXT("Cannot load\nkey file", "无法加载\n密钥文件"));
    } else {
        if(nfc_device_get_protocol(nfc_device) == NfcProtocolMfClassic) {
            const MfClassicData* mf_classic_data =
                nfc_device_get_data(nfc_device, NfcProtocolMfClassic);
            mf_classic_copy(instance->mf_classic_data, mf_classic_data);
            instance->is_unsaved_changes = false;
        } else {
            result = MfcEditorPromptResponseNotMfClassic;
        }
    }

    nfc_device_free(nfc_device);

    return result;
}

bool mfc_editor_save_file(MfcEditorApp* instance) {
    furi_assert(instance);
    furi_assert(instance->file_path);
    furi_assert(instance->mf_classic_data);

    NfcDevice* nfc_device = nfc_device_alloc();

    nfc_device_set_data(nfc_device, NfcProtocolMfClassic, instance->mf_classic_data);

    bool result = nfc_device_save(nfc_device, furi_string_get_cstr(instance->file_path));
    if(!result) {
        dialog_message_show_storage_error(instance->dialogs, MFC_EDITOR_UI_TEXT("Cannot save\nkey file", "无法保存\n密钥文件"));
    }

    nfc_device_free(nfc_device);

    return result;
}

static DialogMessageButton mfc_editor_prompt_should_load_shadow(MfcEditorApp* instance) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, MFC_EDITOR_UI_TEXT("File has modifications", "文件有修改"), 63, 3, AlignCenter, AlignTop);
    dialog_message_set_text(
        message,
        MFC_EDITOR_UI_TEXT("Would you like to edit the\nmodified file (recommended)\nor the original file?", "要编辑修改后的文件\n(推荐)\n还是原始文件?"),
        63,
        31,
        AlignCenter,
        AlignCenter);
    dialog_message_set_buttons(message, MFC_EDITOR_UI_TEXT("Original", "原始"), NULL, MFC_EDITOR_UI_TEXT("Modified", "已修改"));

    DialogMessageButton message_button = dialog_message_show(instance->dialogs, message);

    dialog_message_free(message);

    return message_button;
}

static void mfc_editor_get_shadow_file_path(FuriString* file_path, FuriString* shadow_file_path) {
    furi_assert(file_path);
    furi_assert(shadow_file_path);

    // Remove NFC extension from end of string then append shadow extension
    furi_string_set_n(shadow_file_path, file_path, 0, furi_string_size(file_path) - 4);
    furi_string_cat_printf(shadow_file_path, "%s", NFC_APP_SHADOW_EXTENSION);
}

static bool mfc_editor_file_has_shadow_file(MfcEditorApp* instance, FuriString* file_path) {
    furi_assert(instance);
    furi_assert(file_path);

    FuriString* shadow_file_path = furi_string_alloc();
    mfc_editor_get_shadow_file_path(file_path, shadow_file_path);
    bool has_shadow_file =
        storage_common_exists(instance->storage, furi_string_get_cstr(shadow_file_path));

    furi_string_free(shadow_file_path);

    return has_shadow_file;
}

MfcEditorPromptResponse mfc_editor_prompt_load_file(MfcEditorApp* instance) {
    furi_assert(instance);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, NFC_APP_EXTENSION, &I_Nfc_10px);
    browser_options.base_path = NFC_APP_FOLDER;
    browser_options.hide_dot_files = true;

    MfcEditorPromptResponse result = MfcEditorPromptResponseSuccess;
    if(!dialog_file_browser_show(
           instance->dialogs, instance->file_path, instance->file_path, &browser_options)) {
        result = MfcEditorPromptResponseExitedFile;
    } else {
        if(mfc_editor_file_has_shadow_file(instance, instance->file_path)) {
            DialogMessageButton message_button = mfc_editor_prompt_should_load_shadow(instance);

            if(message_button == DialogMessageButtonRight) {
                // User selected to use shadow file, so replace selected path with that path
                FuriString* shadow_file_path = furi_string_alloc();
                mfc_editor_get_shadow_file_path(instance->file_path, shadow_file_path);
                furi_string_set(instance->file_path, shadow_file_path);
                furi_string_free(shadow_file_path);
            } else if(message_button == DialogMessageButtonBack) {
                result = MfcEditorPromptResponseExitedShadow;
            }
        }

        // Don't load the file if user was prompted for shadow file use but went back
        if(result == MfcEditorPromptResponseSuccess) {
            result = mfc_editor_load_file(instance, instance->file_path);
        }
    }

    return result;
}

bool mfc_editor_warn_risky_operation(MfcEditorApp* instance) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, MFC_EDITOR_UI_TEXT("Risky operation", "危险操作"), 63, 3, AlignCenter, AlignTop);
    dialog_message_set_text(
        message,
        MFC_EDITOR_UI_TEXT("Changing this data may\ninhibit writing to the card\nor could brick the card.", "更改此数据可能会\n阻止写入卡片\n或导致卡片变砖。"),
        63,
        31,
        AlignCenter,
        AlignCenter);
    dialog_message_set_buttons(message, MFC_EDITOR_UI_TEXT("Back", "返回"), MFC_EDITOR_UI_TEXT("Continue", "继续"), NULL);

    DialogMessageButton message_button = dialog_message_show(instance->dialogs, message);

    dialog_message_free(message);

    return message_button == DialogMessageButtonCenter;
}

MfcEditorSaveResponse mfc_editor_warn_unsaved_changes(MfcEditorApp* instance) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, MFC_EDITOR_UI_TEXT("Unsaved changes", "未保存的更改"), 63, 3, AlignCenter, AlignTop);
    dialog_message_set_text(
        message,
        MFC_EDITOR_UI_TEXT("Would you like to save?\nDiscarding your\nchanges is permanent.", "是否保存?\n丢弃更改将无法恢复。"),
        63,
        31,
        AlignCenter,
        AlignCenter);
    dialog_message_set_buttons(message, MFC_EDITOR_UI_TEXT("Discrd", "丢弃"), MFC_EDITOR_UI_TEXT("Save", "保存"), MFC_EDITOR_UI_TEXT("Cancel", "取消"));

    DialogMessageButton message_button = dialog_message_show(instance->dialogs, message);

    dialog_message_free(message);

    if(message_button == DialogMessageButtonCenter) {
        return MfcEditorSaveResponseSave;
    } else if(message_button == DialogMessageButtonLeft) {
        return MfcEditorSaveResponseDiscard;
    } else {
        return MfcEditorSaveResponseCancel;
    }
}

int32_t mfc_editor_app(void* p) {
    UNUSED(p);

    MfcEditorApp* instance = mfc_editor_app_alloc();

    scene_manager_next_scene(instance->scene_manager, MfcEditorSceneFileSelect);

    view_dispatcher_run(instance->view_dispatcher);

    mfc_editor_app_free(instance);

    return 0;
}
