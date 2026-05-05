#include "../saflip.h"

void saflip_scene_log_info_on_enter(void* context) {
    SaflipApp* app = context;

    LogEntry entry =
        app->log[scene_manager_get_scene_state(app->scene_manager, SaflipSceneLogInfo)];

    widget_reset(app->widget);
    FuriString* temp_str = furi_string_alloc();

    // Show lock ID in bold in top-left corner
    furi_string_printf(temp_str, "%d", entry.lock_id);
    widget_add_string_element(
        app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, furi_string_get_cstr(temp_str));

    // Show date/time on the same line, but not bold and on the right
    if(entry.time_is_set) {
        furi_string_printf(
            temp_str,
            "%04d-%02d-%02d %02d:%02d%s\n",
            entry.time.year,
            entry.time.month,
            entry.time.day,
            entry.time.hour,
            entry.time.minute,
            entry.is_dst ? " [D]" : " [S]");
    } else {
        furi_string_printf(temp_str, SAFLIP_UI_TEXT("Clock Not Set", "时钟未设置"));
    }
    widget_add_string_element(
        app->widget, 128, 1, AlignRight, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));

    // Show fields below

    furi_string_printf(temp_str, "%s: %s\n", SAFLIP_UI_TEXT("Lock Prob", "锁故障"), entry.lock_problem ? SAFLIP_UI_TEXT("Yes", "是") : SAFLIP_UI_TEXT("No", "否"));
    widget_add_string_element(
        app->widget, 0, 11, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));

    furi_string_printf(temp_str, "%s: %s\n", SAFLIP_UI_TEXT("Low Batt", "电量低"), entry.low_battery ? SAFLIP_UI_TEXT("Yes", "是") : SAFLIP_UI_TEXT("No", "否"));
    widget_add_string_element(
        app->widget, 0, 21, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));

    furi_string_printf(temp_str, "%s: %s\n", SAFLIP_UI_TEXT("Let Open", "常开"), entry.let_open ? SAFLIP_UI_TEXT("Yes", "是") : SAFLIP_UI_TEXT("No", "否"));
    widget_add_string_element(
        app->widget, 0, 31, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));

    widget_add_line_element(app->widget, 62, 10, 62, 40);

    furi_string_printf(temp_str, "%s: %s\n", SAFLIP_UI_TEXT("Latched", "已锁定"), entry.lock_latched ? SAFLIP_UI_TEXT("Yes", "是") : SAFLIP_UI_TEXT("No", "否"));
    widget_add_string_element(
        app->widget, 66, 11, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));

    furi_string_printf(temp_str, "%s: %s\n", SAFLIP_UI_TEXT("Deadbolt", "反锁"), entry.deadbolt ? SAFLIP_UI_TEXT("On", "开") : SAFLIP_UI_TEXT("Off", "关"));
    widget_add_string_element(
        app->widget, 66, 21, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));

    furi_string_printf(temp_str, "%s: %s\n", SAFLIP_UI_TEXT("New Key", "新钥匙"), entry.new_key ? SAFLIP_UI_TEXT("Yes", "是") : SAFLIP_UI_TEXT("No", "否"));
    widget_add_string_element(
        app->widget, 66, 31, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));

    widget_add_line_element(app->widget, 0, 41, 128, 41);
    char* description = saflok_log_entry_description(entry);
    if(description == NULL) {
        FuriString* unknown_desc =
            furi_string_alloc_printf(SAFLIP_UI_TEXT("Unknown diagnostic code: %d", "未知诊断码: %d"), entry.diagnostic_code);
        widget_add_text_scroll_element(
            app->widget, 0, 44, 128, 20, furi_string_get_cstr(unknown_desc));
        furi_string_free(unknown_desc);
    } else {
        widget_add_text_scroll_element(app->widget, 0, 44, 128, 20, description);
    }

    furi_string_free(temp_str);
    view_dispatcher_switch_to_view(app->view_dispatcher, SaflipViewWidget);
}

bool saflip_scene_log_info_on_event(void* context, SceneManagerEvent event) {
    SaflipApp* app = context;
    bool consumed = false;

    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void saflip_scene_log_info_on_exit(void* context) {
    SaflipApp* app = context;
    widget_reset(app->widget);
}
