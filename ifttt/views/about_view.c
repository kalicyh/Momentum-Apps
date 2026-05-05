#include "about_view.h"
#include <furi.h>
#include <gui/elements.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#ifndef IFTTT_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define IFTTT_UI_TEXT(en, zh) (zh)
#else
#define IFTTT_UI_TEXT(en, zh) (en)
#endif
#endif

struct AboutView {
    View* view;
};

typedef struct {
    bool connected;
} AboutViewModel;

static void about_view_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, IFTTT_UI_TEXT("IFTTT Virtual button", "IFTTT 虚拟按钮"));
    canvas_draw_str_aligned(canvas, 0, 15, AlignLeft, AlignTop, IFTTT_UI_TEXT("Version 0.2", "版本 0.2"));
    canvas_draw_str_aligned(canvas, 0, 50, AlignLeft, AlignTop, IFTTT_UI_TEXT("press back", "按返回键"));
}

AboutView* about_view_alloc() {
    AboutView* about_view = malloc(sizeof(AboutView));
    about_view->view = view_alloc();
    view_set_context(about_view->view, about_view);
    view_allocate_model(about_view->view, ViewModelTypeLocking, sizeof(AboutViewModel));
    view_set_draw_callback(about_view->view, about_view_draw_callback);
    return about_view;
}

void about_view_free(AboutView* about_view) {
    furi_assert(about_view);
    view_free(about_view->view);
    free(about_view);
}

View* about_view_get_view(AboutView* about_view) {
    furi_assert(about_view);
    return about_view->view;
}

void about_view_set_data(AboutView* about_view, bool connected) {
    furi_assert(about_view);
    with_view_model(
        about_view->view, AboutViewModel * model, { model->connected = connected; }, true);
}
