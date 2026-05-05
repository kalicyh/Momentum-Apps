#include "about/about.hpp"

#ifndef flip_telegram_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define flip_telegram_UI_TEXT(en, zh) (zh)
#else
#define flip_telegram_UI_TEXT(en, zh) (en)
#endif
#endif

FlipTelegramAbout::FlipTelegramAbout(ViewDispatcher **viewDispatcher) : widget(nullptr), viewDispatcherRef(viewDispatcher)
{
    easy_flipper_set_widget(&widget, FlipTelegramViewAbout, flip_telegram_UI_TEXT("Flipper Zero Telegram Client\n\n\n\n\nwww.github.com/jblanked", "Flipper Zero 电报客户端\n\n\n\n\nwww.github.com/jblanked"), callbackToSubmenu, viewDispatcherRef);
}

FlipTelegramAbout::~FlipTelegramAbout()
{
    if (widget && viewDispatcherRef && *viewDispatcherRef)
    {
        view_dispatcher_remove_view(*viewDispatcherRef, FlipTelegramViewAbout);
        widget_free(widget);
        widget = nullptr;
    }
}

uint32_t FlipTelegramAbout::callbackToSubmenu(void *context)
{
    UNUSED(context);
    return FlipTelegramViewSubmenu;
}
