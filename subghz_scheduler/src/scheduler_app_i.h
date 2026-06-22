#pragma once

#include "scheduler_app.h"
#include "scenes/scheduler_scene.h"
#include "scheduler_custom_event.h"
#include "subghz_scheduler.h"
#include "views/scheduler_run_view.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <notification/notification_messages.h>
#include <gui/modules/variable_item_list.h>
#include <expansion/expansion.h>
#include <dialogs/dialogs.h>

#ifndef SCHEDULER_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define SCHEDULER_UI_TEXT(en, zh) (zh)
#else
#define SCHEDULER_UI_TEXT(en, zh) (en)
#endif
#endif

#include "scenes/scheduler_scene_settings.h"

struct SchedulerApp {
    Expansion* expansion;
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    DialogsApp* dialogs;
    VariableItemList* var_item_list;
    FuriThread* thread;
    Scheduler* scheduler;
    FuriString* file_path;

    SchedulerRunView* run_view;

    volatile bool is_transmitting;
};

typedef enum {
    SchedulerAppViewVarItemList,
    SchedulerAppViewRunSchedule,
    SchedulerAppViewExitConfirm,
} SchedulerAppView;
