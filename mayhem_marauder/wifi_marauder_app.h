#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_MARAUDER_APP_VERSION "v0.7.2"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define WIFI_MARAUDER_UI_TEXT(en, zh) (zh)
#else
#define WIFI_MARAUDER_UI_TEXT(en, zh) (en)
#endif

typedef struct WifiMarauderApp WifiMarauderApp;

#ifdef __cplusplus
}
#endif
