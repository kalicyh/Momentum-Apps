#ifdef MOMENTUM_UI_LANG_ZH_CN
#define FLIPBOARD_UI_TEXT(en, zh) (zh)
#else
#define FLIPBOARD_UI_TEXT(en, zh) (en)
#endif

#define TAG                         "FlipboardSignal"
#define FLIPBOARD_APP_NAME          "signal"
#define FLIPBOARD_PRIMARY_ITEM_NAME "Flipboard Signal"

// Uncomment the next line to use the LED driver, or comment to use bit-banging the WS2812B LEDs.
#define USE_LED_DRIVER 1

#define FIRMWARE_SUPPORTS_SUBGHZ       1
#define APP_USES_SUBGHZ_PROTOCOL_ITEMS 1

#define ABOUT_TEXT                                     \
    FLIPBOARD_UI_TEXT(                                 \
        "Welcome to the Flipboard\n"                  \
        "Signal v3.12!\n"                             \
        "Quickly send to any device!\n\n"             \
        "Be sure to put your .SUB\n"                  \
        "files in 'SD Card/subghz'\n"                 \
        "action 1: Flip1.sub\n"                       \
        "action 2: Flip2.sub\n"                       \
        "action 4: Flip4.sub\n"                       \
        "action 8: Flip8.sub\n                      v\n" \
        "Name IR file Flipboard.ir\n"                 \
        "action 1: Flip1\n"                           \
        "action 2: Flip2\n"                           \
        "action 4: Flip4\n"                           \
        "action 8: Flip8\n                      v\n"  \
        "Optimized for FlipBoard\n"                   \
        "hardware --\n"                               \
        "see link below to order!\n"                  \
        "Created by @MakeItHackin\n"                  \
        "and @CodeAllNight!\n"                        \
        "https://discord.com/invite/NsjCvqwPAd\n"    \
        "https://youtube.com/@MrDerekJamison\n"       \
        "https://tindie.com/stores/MakeItHackin\n"    \
        "https://x.com/MakeItHackin",                 \
        "欢迎使用 Flipboard\n"                         \
        "Signal v3.12!\n"                             \
        "快速发送到任何设备!\n\n"                      \
        "请将 .SUB 文件放入\n"                        \
        "'SD Card/subghz' 目录\n"                     \
        "动作1: Flip1.sub\n"                          \
        "动作2: Flip2.sub\n"                          \
        "动作4: Flip4.sub\n"                          \
        "动作8: Flip8.sub\n                      v\n" \
        "IR文件命名为 Flipboard.ir\n"                 \
        "动作1: Flip1\n"                              \
        "动作2: Flip2\n"                              \
        "动作4: Flip4\n"                              \
        "动作8: Flip8\n                      v\n"     \
        "专为 FlipBoard 硬件优化\n"                    \
        "详见下方链接购买!\n"                          \
        "作者 @MakeItHackin\n"                        \
        "与 @CodeAllNight!\n"                         \
        "https://discord.com/invite/NsjCvqwPAd\n"    \
        "https://youtube.com/@MrDerekJamison\n"       \
        "https://tindie.com/stores/MakeItHackin\n"    \
        "https://x.com/MakeItHackin")
