#pragma once

#include <gui/gui.h>

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define NRF24SCAN_UI_TEXT(en, zh) (zh)
#else
#define NRF24SCAN_UI_TEXT(en, zh) (en)
#endif
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <toolbox/stream/file_stream.h>
#include <notification/notification_messages.h>

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

struct FOUND {
    uint8_t addr_size;
    uint8_t addr[5];
    uint16_t total;
};

typedef struct {
    Gui* gui;
    FuriMessageQueue* event_queue;
    ViewPort* view_port;
    Storage* storage;
    NotificationApp* notification;
    uint8_t* log_arr;
    struct FOUND* found;
} Nrf24Scan;
