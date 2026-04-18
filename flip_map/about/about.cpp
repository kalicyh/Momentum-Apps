#include "about/about.hpp"
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define FLIP_MAP_UI_TEXT(en, zh) (zh)
#else
#define FLIP_MAP_UI_TEXT(en, zh) (en)
#endif

FlipMapAbout::FlipMapAbout(ViewDispatcher **viewDispatcher) : widget(nullptr), viewDispatcherRef(viewDispatcher)
{
    easy_flipper_set_widget(
        &widget,
        FlipMapViewAbout,
        FLIP_MAP_UI_TEXT(
            "Find Flipper Zero Users\n\n\n\n\nwww.github.com/jblanked",
            "查找 Flipper Zero 用户\n\n\n\n\nwww.github.com/jblanked"),
        callbackToSubmenu,
        viewDispatcherRef);
}

FlipMapAbout::~FlipMapAbout()
{
    if (widget && viewDispatcherRef && *viewDispatcherRef)
    {
        view_dispatcher_remove_view(*viewDispatcherRef, FlipMapViewAbout);
        widget_free(widget);
        widget = nullptr;
    }
}

uint32_t FlipMapAbout::callbackToSubmenu(void *context)
{
    UNUSED(context);
    return FlipMapViewSubmenu;
}
