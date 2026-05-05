#pragma once

typedef struct NfcEinkApp NfcEinkApp;

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define NFC_EINK_UI_TEXT(en, zh) (zh)
#else
#define NFC_EINK_UI_TEXT(en, zh) (en)
#endif