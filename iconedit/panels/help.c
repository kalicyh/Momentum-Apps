#include <gui/canvas.h>
#include <input/input.h>

#include "panels.h"
#include "../iconedit.h"

#define LINE_COUNT 5 // number of lines visible in the panel

typedef enum {
    Normal,
    Bold,
    HorizontalRule,
} LineType;

typedef struct {
    LineType type;
    char* text;
} Line;

const Line BlankLine = {Normal, ""};
const Line Separator = {HorizontalRule, ""};

const Line help_text[] = {
    {Bold, ICONEDIT_UI_TEXT("Draw", "绘图")},
    {Normal, ICONEDIT_UI_TEXT("Pen: OK to toggle pixel", "画笔: OK 切换像素")},
    {Normal, ICONEDIT_UI_TEXT("Line/Circle/Rect:", "线/圆/矩形:")},
    {Normal, ICONEDIT_UI_TEXT("OK to start, then U/D/L/R", "OK 开始，然后上/下/左/右")},
    {Normal, ICONEDIT_UI_TEXT("OK to end. Back to Cancel", "OK 结束，返回取消")},
    BlankLine,
    {Bold, ICONEDIT_UI_TEXT("View / Play", "查看/播放")},
    {Normal, ICONEDIT_UI_TEXT("OK: Pause/Resume", "OK: 暂停/继续")},
    {Normal, ICONEDIT_UI_TEXT("U/D: Zoom In/Out", "上/下: 放大/缩小")},
    {Normal, ICONEDIT_UI_TEXT("L/R: Prev/Next Frame", "左/右: 上/下一帧")},
    BlankLine,
    {Bold, ICONEDIT_UI_TEXT("Open Icon", "打开图标")},
    {Normal, ICONEDIT_UI_TEXT("Only PNG and BMX", "仅支持 PNG 和 BMX")},
    {Normal, ICONEDIT_UI_TEXT("files are supported", "格式的文件")},
    BlankLine,
    {Bold, ICONEDIT_UI_TEXT("Save to", "保存到")},
    {Normal, ICONEDIT_UI_TEXT("Saves files to SDCard", "保存文件到 SD 卡")},
    {Normal, "in /apps_data/iconedit"},
    BlankLine,
    {Bold, ICONEDIT_UI_TEXT("Send to PC", "发送到 PC")},
    {Normal, ICONEDIT_UI_TEXT("Flipper must be connected", "Flipper 需通过 USB")},
    {Normal, ICONEDIT_UI_TEXT("to PC via USB.", "连接到 PC。")},
    BlankLine,
    {Normal, ICONEDIT_UI_TEXT(".C will send C source code", ".C 将发送 C 源代码")},
    {Normal, ICONEDIT_UI_TEXT("to the focused app.", "到当前应用。")},
    BlankLine,
    {Normal, ICONEDIT_UI_TEXT("PNG and BMX require use of", "PNG 和 BMX 需要使用")},
    {Normal, "image_receive.py script"},
    BlankLine,
    {Normal, ICONEDIT_UI_TEXT("More details on github:", "详情见 github:")},
    {Normal, "github.com/rdefeo/iconedit"},
};

const int help_text_num_lines = sizeof(help_text) / sizeof(help_text[0]);
int top_line = 0;

void help_draw(Canvas* canvas, void* context) {
    IconEdit* app = context;

    const int x = 0;
    const int y = 7;
    const int pad = 2; // outside padding between frame and line
    const int line_h = 7;
    // int line_w = 90;
    const int line_pad = 2; // the space between

    for(int l = top_line; l < top_line + LINE_COUNT; l++) {
        Line line = help_text[l];
        switch(line.type) {
        case Normal:
            canvas_set_font(canvas, FontSecondary);
            break;
        case Bold:
            canvas_set_font(canvas, FontPrimary);
            break;
        case HorizontalRule:
            canvas_draw_line(
                canvas,
                x + pad + line_pad + 1,
                y + pad + line_pad + (l - top_line) * (line_h + line_pad * 2) + (line_h / 2),
                x + 90,
                y + pad + line_pad + (l - top_line) * (line_h + line_pad * 2) + (line_h / 2));
            break;
        }
        canvas_draw_str_aligned(
            canvas,
            x + pad + 1,
            y + pad + line_pad + (l - top_line) * (line_h + line_pad * 2),
            AlignLeft,
            AlignTop,
            line.text);
    }

    // scrollbar
    const int cw = canvas_width(canvas);
    const int ch = canvas_height(canvas);

    for(int dy = y; dy < ch; dy += 3) {
        canvas_draw_dot(canvas, cw - 1, dy);
    }
    int sh = (LINE_COUNT * (ch - y)) / help_text_num_lines;
    float yp = (top_line * 1.0) / (help_text_num_lines - LINE_COUNT);
    int so = (ch - y - sh) * yp;
    canvas_draw_line(canvas, cw - 1, y + so, cw - 1, y + so + sh);
    if(app->panel == Panel_Help) {
        canvas_draw_line(canvas, cw - 1 - 1, y + so, cw - 1 - 1, y + so + sh);
    }
}

bool help_input(InputEvent* event, void* context) {
    IconEdit* app = context;
    bool consumed = true;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyUp: {
            if(top_line == 0) {
                app->panel = Panel_TabBar;
            } else {
                top_line--;
            }
            break;
        }
        case InputKeyDown: {
            if(top_line < help_text_num_lines - LINE_COUNT) {
                top_line++;
            }
            break;
        }
        case InputKeyBack:
            app->panel = Panel_TabBar;
            break;
        default:
            break;
        }
    }
    return consumed;
}
