#pragma once

#ifndef CHIEF_COOKER_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define CHIEF_COOKER_UI_TEXT(en, zh) (zh)
#else
#define CHIEF_COOKER_UI_TEXT(en, zh) (en)
#endif
#endif

#include "lib/String.hpp"
#include "lib/ui/view/UiView.hpp"
#include "lib/ui/view/ProgressbarPopupUiView.hpp"

class BatchTransmissionScreen {
private:
    ProgressbarPopupUiView* popup;
    String statusStr;

public:
    BatchTransmissionScreen(int pagersTotal) {
        popup = new ProgressbarPopupUiView(CHIEF_COOKER_UI_TEXT("Transmitting...", "传输中..."));
        SetProgress(0, pagersTotal);
        popup->SetOnDestroyHandler(HANDLER(&BatchTransmissionScreen::destroy));
    }

    void SetProgress(int pagerNum, int pagersTotal) {
        float progressValue = (float)pagerNum / pagersTotal;
        popup->SetProgress(statusStr.format(CHIEF_COOKER_UI_TEXT("Pager %d / %d", "呼机 %d / %d"), pagerNum, pagersTotal), progressValue);
    }

private:
    void destroy() {
        delete this;
    }

public:
    UiView* GetView() {
        return popup;
    }
};
