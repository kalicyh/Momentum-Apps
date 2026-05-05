#pragma once

#define SCREEN_WIDTH (128)
#define SCREEN_HEIGHT (64)
#define SCREEN_WIDTH_CENTER (SCREEN_WIDTH >> 1)
#define SCREEN_HEIGHT_CENTER (SCREEN_HEIGHT >> 1)

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define TOTP_UI_TEXT(en, zh) (zh)
#else
#define TOTP_UI_TEXT(en, zh) (en)
#endif
