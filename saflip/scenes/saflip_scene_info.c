#include "../saflip.h"

static const char* card_levels[] = {
    SAFLIP_UI_TEXT("Guest Key", "宾客钥匙"),
    SAFLIP_UI_TEXT("Connectors", "连接器"),
    SAFLIP_UI_TEXT("Suite", "套房"),
    SAFLIP_UI_TEXT("Limited Use", "受限使用"),
    SAFLIP_UI_TEXT("Failsafe", "故障安全"),
    SAFLIP_UI_TEXT("Inhibit", "禁止"),
    SAFLIP_UI_TEXT("Pool/Meeting Master", "泳池/会议主卡"),
    SAFLIP_UI_TEXT("Housekeeping", "客房服务"),
    SAFLIP_UI_TEXT("Floor Key", "楼层钥匙"),
    SAFLIP_UI_TEXT("Section Key", "区域钥匙"),
    SAFLIP_UI_TEXT("Rooms Master", "房间主卡"),
    SAFLIP_UI_TEXT("Grand Master", "总主卡"),
    SAFLIP_UI_TEXT("Emergency", "紧急"),
    SAFLIP_UI_TEXT("Electronic Lockout", "电子锁定"),
    SAFLIP_UI_TEXT("Secondary Programming Key", "副编程钥匙"),
    SAFLIP_UI_TEXT("Primary Programming Key", "主编程钥匙"),
};

static const char* card_types[] = {
    SAFLIP_UI_TEXT("Standard Key", "标准钥匙"),
    SAFLIP_UI_TEXT("Resequencing Key", "重排序钥匙"),
    SAFLIP_UI_TEXT("Block Key", "锁定钥匙"),
    SAFLIP_UI_TEXT("Unblock Key", "解锁钥匙"),
    SAFLIP_UI_TEXT("Change Checkout Date", "修改退房日期"),
    SAFLIP_UI_TEXT("Check Out a Key", "退房钥匙"),
    SAFLIP_UI_TEXT("Cancel Prereg Key", "取消预登记钥匙"),
    SAFLIP_UI_TEXT("Check In Prereg Key", "入住预登记钥匙"),
    SAFLIP_UI_TEXT("Cancel-A-Key-ID", "取消钥匙ID"),
    SAFLIP_UI_TEXT("Change Room Key", "换房钥匙"),
};

static const char* ppk_card_types[] = {
    SAFLIP_UI_TEXT("Standard Key", "标准钥匙"),
    SAFLIP_UI_TEXT("LED Diagnostic", "LED诊断"),
    SAFLIP_UI_TEXT("Dis/Enable E2 Changes", "启用/禁用E2修改"),
    SAFLIP_UI_TEXT("Erase Lock (E2) Memory", "擦除锁(E2)存储"),
    SAFLIP_UI_TEXT("Battery Disconect", "电池断开"),
    SAFLIP_UI_TEXT("Display Key", "显示钥匙"),
    SAFLIP_UI_TEXT("Card Type: 6", "卡类型: 6"),
    SAFLIP_UI_TEXT("Card Type: 7", "卡类型: 7"),
    SAFLIP_UI_TEXT("Card Type: 8", "卡类型: 8"),
    SAFLIP_UI_TEXT("Card Type: 9", "卡类型: 9"),
};

void saflip_scene_info_widget_callback(GuiButtonType button, InputType type, void* context) {
    SaflipApp* app = context;

    if(button == GuiButtonTypeLeft) {
        scene_manager_set_scene_state(app->scene_manager, SaflipSceneVariableKeys, 0);
        scene_manager_next_scene(app->scene_manager, SaflipSceneVariableKeys);
    } else if(button == GuiButtonTypeCenter) {
        scene_manager_set_scene_state(app->scene_manager, SaflipSceneLog, 0);
        scene_manager_next_scene(app->scene_manager, SaflipSceneLog);
    } else if(button == GuiButtonTypeRight) {
        scene_manager_set_scene_state(app->scene_manager, SaflipSceneOptions, 0);
        scene_manager_next_scene(app->scene_manager, SaflipSceneOptions);
    }

    UNUSED(type);
}

void saflip_scene_info_on_enter(void* context) {
    SaflipApp* app = context;

    widget_reset(app->widget);
    FuriString* temp_str = furi_string_alloc();

    // Show card format in bold in top-left corner
    switch(app->data->format) {
    case SaflipFormatMifareClassic:
        furi_string_printf(temp_str, "MFC");
        break;
    default:
        furi_string_printf(temp_str, SAFLIP_UI_TEXT("Unknown Format", "未知格式"));
        break;
    }
    widget_add_string_element(
        app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, furi_string_get_cstr(temp_str));

    // Show UID on the same line, but not bold and on the right
    furi_string_reset(temp_str);
    for(size_t i = 0; i < app->uid_len; i++) {
        if(i) furi_string_cat_printf(temp_str, ":");
        furi_string_cat_printf(temp_str, "%02X", app->uid[i]);
    }
    widget_add_string_element(
        app->widget, 128, 1, AlignRight, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));

    // Show fields below
    furi_string_reset(temp_str);

    if(app->data->card_level >= COUNT_OF(card_levels))
        furi_string_cat_printf(temp_str, SAFLIP_UI_TEXT("Card Level: %d\n", "卡级别: %d\n"), app->data->card_level);
    else
        furi_string_cat_printf(temp_str, "%s\n", card_levels[app->data->card_level]);

    if(app->data->card_type >= COUNT_OF(card_types)) {
        furi_string_cat_printf(temp_str, SAFLIP_UI_TEXT("Card Type: %d\n", "卡类型: %d\n"), app->data->card_type);
    } else {
        if(app->data->card_level == 15)
            furi_string_cat_printf(temp_str, "%s\n", ppk_card_types[app->data->card_type]);
        else
            furi_string_cat_printf(temp_str, "%s\n", card_types[app->data->card_type]);
    }

    furi_string_cat_printf(temp_str, SAFLIP_UI_TEXT("Property Number: %d\n", "物业号: %d\n"), app->data->property_id);

    furi_string_cat_printf(temp_str, SAFLIP_UI_TEXT("Card ID: %d\n", "卡号: %d\n"), app->data->card_id);
    furi_string_cat_printf(temp_str, SAFLIP_UI_TEXT("Opening Key: %s\n", "开门钥匙: %s\n"), app->data->opening_key ? SAFLIP_UI_TEXT("Yes", "是") : SAFLIP_UI_TEXT("No", "否"));
    furi_string_cat_printf(temp_str, SAFLIP_UI_TEXT("Lock ID: %d\n", "锁号: %d\n"), app->data->lock_id);
    furi_string_cat_printf(temp_str, SAFLIP_UI_TEXT("Pass #/Areas: %d\n", "通行证/区域: %d\n"), app->data->pass_number);

    furi_string_cat_printf(
        temp_str, SAFLIP_UI_TEXT("Seq. & Combination: %d\n", "序号与组合: %d\n"), app->data->sequence_and_combination);
    furi_string_cat_printf(
        temp_str, SAFLIP_UI_TEXT("Override Deadbolt: %s\n", "覆盖反锁: %s\n"), app->data->deadbolt_override ? SAFLIP_UI_TEXT("Yes", "是") : SAFLIP_UI_TEXT("No", "否"));

    furi_string_cat_printf(
        temp_str,
        SAFLIP_UI_TEXT("Can be used: %c %c %c %c %c %c %c\n", "可用: %c %c %c %c %c %c %c\n"),
        (app->data->restricted_days & 0b0000001) ? '-' : 'S',
        (app->data->restricted_days & 0b0000010) ? '-' : 'M',
        (app->data->restricted_days & 0b0000100) ? '-' : 'T',
        (app->data->restricted_days & 0b0001000) ? '-' : 'W',
        (app->data->restricted_days & 0b0010000) ? '-' : 'T',
        (app->data->restricted_days & 0b0100000) ? '-' : 'F',
        (app->data->restricted_days & 0b1000000) ? '-' : 'S');

    furi_string_cat_printf(
        temp_str,
        SAFLIP_UI_TEXT("Valid: %04d-%02d-%02d %02d:%02d\n", "有效期: %04d-%02d-%02d %02d:%02d\n"),
        app->data->creation.year,
        app->data->creation.month,
        app->data->creation.day,
        app->data->creation.hour,
        app->data->creation.minute);
    furi_string_cat_printf(
        temp_str,
        SAFLIP_UI_TEXT("Expires: %04d-%02d-%02d %02d:%02d\n", "过期: %04d-%02d-%02d %02d:%02d\n"),
        app->data->expire.year,
        app->data->expire.month,
        app->data->expire.day,
        app->data->expire.hour,
        app->data->expire.minute);

    furi_string_cat_printf(
        temp_str,
        SAFLIP_UI_TEXT("Contains %d variable %s\n", "包含 %d 个可变%s\n"),
        app->variable_keys,
        app->variable_keys == 1 ? SAFLIP_UI_TEXT("key", "密钥") : SAFLIP_UI_TEXT("keys", "密钥"));
    furi_string_cat_printf(
        temp_str,
        SAFLIP_UI_TEXT("Contains %d log %s", "包含 %d 条日志%s"),
        app->log_entries,
        app->log_entries == 1 ? SAFLIP_UI_TEXT("entry", "记录") : SAFLIP_UI_TEXT("entries", "记录"));

    widget_add_text_scroll_element(app->widget, 0, 10, 128, 40, furi_string_get_cstr(temp_str));

    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, SAFLIP_UI_TEXT("V.Keys", "可变密钥"), saflip_scene_info_widget_callback, app);
    if(app->log_entries)
        widget_add_button_element(
            app->widget, GuiButtonTypeCenter, SAFLIP_UI_TEXT("Log", "日志"), saflip_scene_info_widget_callback, app);
    widget_add_button_element(
        app->widget, GuiButtonTypeRight, SAFLIP_UI_TEXT("Options", "选项"), saflip_scene_info_widget_callback, app);

    furi_string_free(temp_str);
    view_dispatcher_switch_to_view(app->view_dispatcher, SaflipViewWidget);
}

bool saflip_scene_info_on_event(void* context, SceneManagerEvent event) {
    SaflipApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        // We don't want to go back to the Read Card scene
        uint32_t scenes[] = {
            SaflipSceneEdit,
            SaflipSceneFileSelect,
            SaflipSceneStart,
        };

        consumed = scene_manager_search_and_switch_to_previous_scene_one_of(
            app->scene_manager, scenes, COUNT_OF(scenes));
    }

    return consumed;
}

void saflip_scene_info_on_exit(void* context) {
    SaflipApp* app = context;
    widget_reset(app->widget);
}
