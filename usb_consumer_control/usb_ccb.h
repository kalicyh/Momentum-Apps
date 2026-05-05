#pragma once

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define USB_CCB_UI_TEXT(en, zh) (zh)
#else
#define USB_CCB_UI_TEXT(en, zh) (en)
#endif

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <notification/notification.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include "views/usb_ccb_about.h"
#include "views/usb_ccb_help.h"
#include "views/usb_ccb_start.h"

typedef struct {
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    DialogEx* dialog;
    UsbCcbAbout* usb_ccb_about;
    UsbCcbHelp* usb_ccb_help;
    UsbCcbStart* usb_ccb_start;
    uint32_t view_id;
} UsbCcb;

typedef enum {
    UsbCcbViewSubmenu,
    UsbCcbViewAbout,
    UsbCcbViewHelp,
    UsbCcbViewStart,
    UsbCcbViewExitConfirm,
} UsbCcbView;
