#pragma once

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define AIRMOUSE_UI_TEXT(en, zh) (zh)
#else
#define AIRMOUSE_UI_TEXT(en, zh) (en)
#endif

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>

#include "views/bt_mouse.h"
#include "views/usb_mouse.h"
#include "views/calibration.h"

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    DialogEx* dialog;
    DialogEx* error_dialog;
    BtMouse* bt_mouse;
    UsbMouse* usb_mouse;
    Calibration* calibration;
    uint32_t view_id;
} AirMouse;

typedef enum {
    AirMouseViewSubmenu,
    AirMouseViewBtMouse,
    AirMouseViewUsbMouse,
    AirMouseViewCalibration,
    AirMouseViewExitConfirm,
    AirMouseViewError,
} AirMouseView;
