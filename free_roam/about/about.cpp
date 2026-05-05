#include "about/about.hpp"

#ifndef free_roam_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define free_roam_UI_TEXT(en, zh) (zh)
#else
#define free_roam_UI_TEXT(en, zh) (en)
#endif
#endif

FreeRoamAbout::FreeRoamAbout()
{
    // nothing to do
}

FreeRoamAbout::~FreeRoamAbout()
{
    free();
}

uint32_t FreeRoamAbout::callbackToSubmenu(void *context)
{
    UNUSED(context);
    return FreeRoamViewSubmenu;
}

bool FreeRoamAbout::init(ViewDispatcher **viewDispatcher, void *appContext)
{
    viewDispatcherRef = viewDispatcher;
    this->appContext = appContext;
    return easy_flipper_set_widget(&widget, FreeRoamViewAbout, free_roam_UI_TEXT("3D Multiplayer Game for the\nFlipper Zero and Picoware\ncreated by JBlanked\n\n\nwww.github.com/jblanked", "Flipper Zero 和 Picoware\n的 3D 多人游戏\n作者: JBlanked\n\n\nwww.github.com/jblanked"), callbackToSubmenu, viewDispatcherRef);
}

void FreeRoamAbout::free()
{
    if (widget && viewDispatcherRef && *viewDispatcherRef)
    {
        view_dispatcher_remove_view(*viewDispatcherRef, FreeRoamViewAbout);
        widget_free(widget);
        widget = nullptr;
    }
}
