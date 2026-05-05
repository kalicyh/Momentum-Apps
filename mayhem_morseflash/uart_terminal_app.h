#pragma once

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define MORSEFLASH_UI_TEXT(en, zh) (zh)
#else
#define MORSEFLASH_UI_TEXT(en, zh) (en)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UART_TerminalApp UART_TerminalApp;

#ifdef __cplusplus
}
#endif
