#ifdef MOMENTUM_UI_LANG_ZH_CN
#define FLIPBOARD_UI_TEXT(en, zh) (zh)
#else
#define FLIPBOARD_UI_TEXT(en, zh) (en)
#endif

#define TAG "FlipBoardSimon"
#define FLIPBOARD_APP_NAME "simon"
#define FLIPBOARD_PRIMARY_ITEM_NAME "Play Simon"

// Uncomment the next line to use the LED driver, or comment to use bit-banging the WS2812B LEDs.
#define USE_LED_DRIVER 1

#define ABOUT_TEXT                                     \
    FLIPBOARD_UI_TEXT(                                 \
        "Welcome to the Flipboard\n"                  \
        "Simon v1.10!\n\n"                            \
        "Optimized for FlipBoard \n"                  \
        "hardware --\n"                               \
        "see link below to order!\n"                  \
        "Created by @MakeItHackin\n"                  \
        "and @CodeAllNight!\n"                        \
        "https://discord.com/invite/NsjCvqwPAd\n"    \
        "https://youtube.com/@MrDerekJamison\n"       \
        "https://tindie.com/stores/MakeItHackin\n"    \
        "https://x.com/MakeItHackin",                 \
        "欢迎使用 Flipboard\n"                         \
        "Simon v1.10!\n\n"                            \
        "专为 FlipBoard 硬件优化\n"                    \
        "详见下方链接购买!\n"                          \
        "作者 @MakeItHackin\n"                        \
        "与 @CodeAllNight!\n"                         \
        "https://discord.com/invite/NsjCvqwPAd\n"    \
        "https://youtube.com/@MrDerekJamison\n"       \
        "https://tindie.com/stores/MakeItHackin\n"    \
        "https://x.com/MakeItHackin")
