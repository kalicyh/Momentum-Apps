#include "connect_view.h"

void draw_connect_view(Canvas* canvas) {
    canvas_draw_str(canvas, 1, 10, PC_MONITOR_UI_TEXT("Waiting for connection...", "等待连接..."));

    canvas_draw_str(canvas, 1, 40, PC_MONITOR_UI_TEXT("Download back-end", "下载 PC 端"));
    canvas_draw_str(canvas, 1, 50, PC_MONITOR_UI_TEXT("app for your PC from:", "程序请前往:"));

    canvas_draw_icon(canvas, 128-34, 64-34, &I_qr_33px);
}
