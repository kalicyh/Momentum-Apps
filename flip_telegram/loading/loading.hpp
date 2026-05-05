#pragma once

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define flip_telegram_UI_TEXT(en, zh) (zh)
#else
#define flip_telegram_UI_TEXT(en, zh) (en)
#endif

#include <gui/gui.h>

class Loading
{
public:
    Loading(Canvas *canvas);
    //
    void animate();
    void stop();
    //
    void setText(const char *text) { currentText = text; }
    //
    uint32_t getTimeElapsed() { return timeElapsed; }

private:
    Canvas *canvas = nullptr;
    void drawSpinner();
    uint32_t spinnerPosition;
    uint32_t timeElapsed;
    uint32_t timeStart;
    bool animating = false;
    const char *currentText = flip_telegram_UI_TEXT("Loading...", "加载中...");
};