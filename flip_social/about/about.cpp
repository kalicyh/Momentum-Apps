#include "about/about.hpp"
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define FLIP_SOCIAL_UI_TEXT(en, zh) (zh)
#else
#define FLIP_SOCIAL_UI_TEXT(en, zh) (en)
#endif

FlipSocialAbout::FlipSocialAbout(ViewDispatcher **viewDispatcher) : widget(nullptr), viewDispatcherRef(viewDispatcher)
{
    easy_flipper_set_widget(
        &widget,
        FlipSocialViewAbout,
        FLIP_SOCIAL_UI_TEXT(
            "Welcome to FlipSocial\n---\nThe social media app for\nFlipper Zero, created by\nJBlanked\n\nwww.github.com/jblanked\n---\nPress BACK to return.",
            "欢迎使用 FlipSocial\n---\n这是为 Flipper Zero 打造的\n社交媒体应用，作者 JBlanked\n\nwww.github.com/jblanked\n---\n按 BACK 返回。"),
        callbackToSubmenu,
        viewDispatcherRef);
}

FlipSocialAbout::~FlipSocialAbout()
{
    if (widget && viewDispatcherRef && *viewDispatcherRef)
    {
        view_dispatcher_remove_view(*viewDispatcherRef, FlipSocialViewAbout);
        widget_free(widget);
        widget = nullptr;
    }
}

uint32_t FlipSocialAbout::callbackToSubmenu(void *context)
{
    UNUSED(context);
    return FlipSocialViewSubmenu;
}
