#include "../picopass_i.h"
#include <furi_hal.h>

enum SubmenuIndex {
    SubmenuIndexRead,
    SubmenuIndexSaved,
    SubmenuIndexCreate,
    SubmenuIndexLoclass,
    SubmenuIndexNRMAC,
    SubmenuIndexCleanMKF,
    SubmenuIndexAcknowledgements,
    SubmenuIndexKeygenAttack,
};

void picopass_scene_start_submenu_callback(void* context, uint32_t index) {
    Picopass* picopass = context;
    view_dispatcher_send_custom_event(picopass->view_dispatcher, index);
}
void picopass_scene_start_on_enter(void* context) {
    Picopass* picopass = context;
    // Reset on enter
    picopass->nr_mac_type = ManualNRMAC;

    Submenu* submenu = picopass->submenu;
    submenu_add_item(
        submenu,
        PICOPASS_UI_TEXT("Read Card", "读卡"),
        SubmenuIndexRead,
        picopass_scene_start_submenu_callback,
        picopass);
    submenu_add_item(
        submenu,
        PICOPASS_UI_TEXT("Saved", "已保存"),
        SubmenuIndexSaved,
        picopass_scene_start_submenu_callback,
        picopass);
    submenu_add_item(
        submenu,
        PICOPASS_UI_TEXT("Create", "创建"),
        SubmenuIndexCreate,
        picopass_scene_start_submenu_callback,
        picopass);
    submenu_add_item(
        submenu,
        PICOPASS_UI_TEXT("Loclass", "Loclass"),
        SubmenuIndexLoclass,
        picopass_scene_start_submenu_callback,
        picopass);
    submenu_add_item(
        submenu,
        PICOPASS_UI_TEXT("Clean MKF", "清理 MKF"),
        SubmenuIndexCleanMKF,
        picopass_scene_start_submenu_callback,
        picopass);
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        submenu_add_item(
            submenu, "NR-MAC", SubmenuIndexNRMAC, picopass_scene_start_submenu_callback, picopass);
    }
    submenu_add_item(
        submenu,
        PICOPASS_UI_TEXT("Acknowledgements", "致谢"),
        SubmenuIndexAcknowledgements,
        picopass_scene_start_submenu_callback,
        picopass);
    submenu_add_item(
        submenu,
        PICOPASS_UI_TEXT("Elite Keygen Attack", "Elite 密钥生成攻击"),
        SubmenuIndexKeygenAttack,
        picopass_scene_start_submenu_callback,
        picopass);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(picopass->scene_manager, PicopassSceneStart));
    picopass_device_clear(picopass->dev);
    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewMenu);
}

bool picopass_scene_start_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexRead) {
            scene_manager_set_scene_state(
                picopass->scene_manager, PicopassSceneStart, SubmenuIndexRead);
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneEliteDictAttack);
            consumed = true;
        } else if(event.event == SubmenuIndexSaved) {
            // Explicitly save state so that the correct item is
            // reselected if the user cancels loading a file.
            scene_manager_set_scene_state(
                picopass->scene_manager, PicopassSceneStart, SubmenuIndexSaved);
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneFileSelect);
            consumed = true;
        } else if(event.event == SubmenuIndexLoclass) {
            scene_manager_set_scene_state(
                picopass->scene_manager, PicopassSceneStart, SubmenuIndexLoclass);
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneLoclass);
            consumed = true;
        } else if(event.event == SubmenuIndexCreate) {
            scene_manager_set_scene_state(
                picopass->scene_manager, PicopassSceneStart, SubmenuIndexCreate);
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneCreate);
            consumed = true;
        } else if(event.event == SubmenuIndexCleanMKF) {
            scene_manager_set_scene_state(
                picopass->scene_manager, PicopassSceneStart, SubmenuIndexCleanMKF);
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneCleanCard);
            consumed = true;
        } else if(event.event == SubmenuIndexNRMAC) {
            picopass->nr_mac_type = AutoNRMAC;
            scene_manager_set_scene_state(
                picopass->scene_manager, PicopassSceneStart, SubmenuIndexNRMAC);
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneEliteDictAttack);
            consumed = true;
        } else if(event.event == SubmenuIndexAcknowledgements) {
            scene_manager_set_scene_state(
                picopass->scene_manager, PicopassSceneStart, SubmenuIndexAcknowledgements);
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneAcknowledgements);
            consumed = true;
        } else if(event.event == SubmenuIndexKeygenAttack) {
            scene_manager_set_scene_state(
                picopass->scene_manager, PicopassSceneStart, SubmenuIndexKeygenAttack);
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneEliteKeygenAttack);
            consumed = true;
        }
    }

    return consumed;
}

void picopass_scene_start_on_exit(void* context) {
    Picopass* picopass = context;
    submenu_reset(picopass->submenu);
}
