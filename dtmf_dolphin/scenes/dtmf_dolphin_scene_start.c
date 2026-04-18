#include "../dtmf_dolphin_i.h"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define DTMF_UI_TEXT(en, zh) (zh)
#else
#define DTMF_UI_TEXT(en, zh) (en)
#endif

static void dtmf_dolphin_scene_start_main_menu_enter_callback(void* context, uint32_t index) {
    DTMFDolphinApp* app = context;
    uint8_t cust_event = 255;
    switch(index) {
    case 0:
        cust_event = DTMFDolphinEventStartDialer;
        break;
    case 1:
        cust_event = DTMFDolphinEventStartBluebox;
        break;
    case 2:
        cust_event = DTMFDolphinEventStartRedboxUS;
        break;
    case 3:
        cust_event = DTMFDolphinEventStartRedboxUK;
        break;
    case 4:
        cust_event = DTMFDolphinEventStartRedboxCA;
        break;
    case 5:
        cust_event = DTMFDolphinEventStartMisc;
        break;
    default:
        return;
    }

    view_dispatcher_send_custom_event(app->view_dispatcher, cust_event);
}

void dtmf_dolphin_scene_start_on_enter(void* context) {
    DTMFDolphinApp* app = context;
    VariableItemList* var_item_list = app->main_menu_list;

    // VariableItem* item;
    variable_item_list_set_enter_callback(
        var_item_list, dtmf_dolphin_scene_start_main_menu_enter_callback, app);

    variable_item_list_add(var_item_list, DTMF_UI_TEXT("Dialer", "拨号器"), 0, NULL, context);
    variable_item_list_add(var_item_list, DTMF_UI_TEXT("Bluebox", "蓝盒"), 0, NULL, context);
    variable_item_list_add(var_item_list, DTMF_UI_TEXT("Redbox (US)", "红盒(美版)"), 0, NULL, context);
    variable_item_list_add(var_item_list, DTMF_UI_TEXT("Redbox (UK)", "红盒(英版)"), 0, NULL, context);
    variable_item_list_add(var_item_list, DTMF_UI_TEXT("Redbox (CA)", "红盒(加拿大)"), 0, NULL, context);
    variable_item_list_add(var_item_list, DTMF_UI_TEXT("Misc", "杂项"), 0, NULL, context);

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, DTMFDolphinSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, DTMFDolphinViewMainMenu);
}

bool dtmf_dolphin_scene_start_on_event(void* context, SceneManagerEvent event) {
    DTMFDolphinApp* app = context;
    UNUSED(app);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        uint8_t sc_state;

        switch(event.event) {
        case DTMFDolphinEventStartDialer:
            sc_state = DTMFDolphinSceneStateDialer;
            break;
        case DTMFDolphinEventStartBluebox:
            sc_state = DTMFDolphinSceneStateBluebox;
            break;
        case DTMFDolphinEventStartRedboxUS:
            sc_state = DTMFDolphinSceneStateRedboxUS;
            break;
        case DTMFDolphinEventStartRedboxUK:
            sc_state = DTMFDolphinSceneStateRedboxUK;
            break;
        case DTMFDolphinEventStartRedboxCA:
            sc_state = DTMFDolphinSceneStateRedboxCA;
            break;
        case DTMFDolphinEventStartMisc:
            sc_state = DTMFDolphinSceneStateMisc;
            break;
        default:
            return consumed;
        }
        scene_manager_set_scene_state(app->scene_manager, DTMFDolphinSceneDialer, sc_state);
        scene_manager_next_scene(app->scene_manager, DTMFDolphinSceneDialer);

        consumed = true;
    }
    return consumed;
}

void dtmf_dolphin_scene_start_on_exit(void* context) {
    DTMFDolphinApp* app = context;
    variable_item_list_reset(app->main_menu_list);
}
