#include "status_view.h"

void draw_status_view(Canvas* canvas, void* ctx) {
    PcMonitorApp* app = ctx;

    canvas_draw_str_aligned(
        canvas,
        64,
        32,
        AlignCenter,
        AlignCenter,
        app->bt_state == BtStateChecking ? PC_MONITOR_UI_TEXT("Checking BLE...", "检查 BLE...") :
        app->bt_state == BtStateInactive ? PC_MONITOR_UI_TEXT("BLE inactive!", "BLE 未启用!") :
        app->bt_state == BtStateLost ?
            PC_MONITOR_UI_TEXT("Connection lost!", "连接已断开!") :
            PC_MONITOR_UI_TEXT("No data!", "暂无数据!"));
}
