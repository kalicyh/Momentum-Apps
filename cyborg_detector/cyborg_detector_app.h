#pragma once

#include <gui/gui.h>
#include <gui/view_port.h>
#include <furi_hal_nfc.h>

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define CYBORG_UI_TEXT(en, zh) (zh)
#else
#define CYBORG_UI_TEXT(en, zh) (en)
#endif

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    bool running;
    bool field_active;
} CyborgDetectorApp;

CyborgDetectorApp* cyborg_detector_app_alloc();
void cyborg_detector_app_free(CyborgDetectorApp* app);
int32_t cyborg_detector_app(void* p);
