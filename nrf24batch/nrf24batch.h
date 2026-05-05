#pragma once

#include <gui/gui.h>

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define NRF24BATCH_UI_TEXT(en, zh) (zh)
#else
#define NRF24BATCH_UI_TEXT(en, zh) (en)
#endif
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <toolbox/stream/file_stream.h>
#include <notification/notification_messages.h>
#include <power/power_service/power.h>
#include <power/power_service/power_i.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    FuriMutex* mutex;
} PluginState;

typedef struct {
    Gui* gui;
    Storage* storage;
    NotificationApp* notification;
    PluginState* plugin_state;
} nRF24Batch;
