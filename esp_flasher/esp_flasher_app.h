#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define ESP_FLASHER_UI_TEXT(en, zh) (zh)
#else
#define ESP_FLASHER_UI_TEXT(en, zh) (en)
#endif

#define ESP_FLASHER_APP_VERSION "v1.8"

typedef struct EspFlasherApp EspFlasherApp;

#ifdef __cplusplus
}
#endif
