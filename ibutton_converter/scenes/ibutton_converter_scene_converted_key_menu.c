#include "../ibutton_converter_i.h"

enum SubmenuIndex {
    SubmenuIndexSave,
    SubmenuIndexInfo,
};

void ibutton_converter_scene_converted_key_menu_on_enter(void* context) {
    iButtonConverter* ibutton_converter = context;
    Submenu* submenu = ibutton_converter->submenu;

    submenu_add_item(
        submenu,
        IBUTTON_CONVERTER_UI_TEXT("Save", "保存"),
        SubmenuIndexSave,
        ibutton_converter_submenu_callback,
        ibutton_converter);
    submenu_add_item(
        submenu,
        IBUTTON_CONVERTER_UI_TEXT("Info", "信息"),
        SubmenuIndexInfo,
        ibutton_converter_submenu_callback,
        ibutton_converter);

    view_dispatcher_switch_to_view(
        ibutton_converter->view_dispatcher, iButtonConverterViewSubmenu);
}

bool ibutton_converter_scene_converted_key_menu_on_event(void* context, SceneManagerEvent event) {
    iButtonConverter* ibutton_converter = context;
    SceneManager* scene_manager = ibutton_converter->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == SubmenuIndexSave) {
            scene_manager_next_scene(scene_manager, iButtonConverterSceneSaveName);
        } else if(event.event == SubmenuIndexInfo) {
            scene_manager_next_scene(scene_manager, iButtonConverterSceneInfo);
        }
    }

    return consumed;
}

void ibutton_converter_scene_converted_key_menu_on_exit(void* context) {
    iButtonConverter* ibutton_converter = context;
    submenu_reset(ibutton_converter->submenu);
}
