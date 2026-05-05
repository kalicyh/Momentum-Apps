#pragma once

#ifndef CHIEF_COOKER_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define CHIEF_COOKER_UI_TEXT(en, zh) (zh)
#else
#define CHIEF_COOKER_UI_TEXT(en, zh) (en)
#endif
#endif

#include "lib/String.hpp"
#include "app/AppConfig.hpp"
#include "app/pager/data/StoredPagerData.hpp"
#include "app/pager/decoder/PagerDecoder.hpp"
#include "app/pager/protocol/PagerProtocol.hpp"
#include "lib/hardware/subghz/SubGhzModule.hpp"
#include "lib/ui/view/UiView.hpp"
#include "lib/ui/view/SubMenuUiView.hpp"
#include "app/screen/BatchTransmissionScreen.hpp"
#include "lib/FlipperDolphin.hpp"
#include "lib/ui/UiManager.hpp"

class PagerActionsScreen {
private:
    AppConfig* config;
    SubMenuUiView* submenu;
    PagerDecoder* decoder;
    PagerProtocol* protocol;
    SubGhzModule* subghz;
    PagerDataGetter getPager;
    BatchTransmissionScreen* batchTransmissionScreen;

    String headerStr;
    String resendToAllStr;
    String resendToCurrentStr;
    String** actionsStrings;

    uint32_t currentBatchFrequency;
    uint32_t currentPager = 0;
    bool transmittingBatch = false;

public:
    PagerActionsScreen(
        AppConfig* config,
        PagerDataGetter pagerGetter,
        PagerDecoder* decoder,
        PagerProtocol* protocol,
        SubGhzModule* subghz
    ) {
        this->config = config;
        this->getPager = pagerGetter;
        this->decoder = decoder;
        this->protocol = protocol;
        this->subghz = subghz;

        StoredPagerData* pager = getPager();
        PagerAction currentAction = decoder->GetAction(pager->data);
        uint8_t actionValue = decoder->GetActionValue(pager->data);
        uint16_t stationNum = decoder->GetStation(pager->data);
        uint16_t pagerNum = decoder->GetPager(pager->data);

        submenu = new SubMenuUiView(headerStr.format(CHIEF_COOKER_UI_TEXT("Station %d actions", "电台 %d 操作"), stationNum));
        submenu->SetOnDestroyHandler(HANDLER(&PagerActionsScreen::destroy));
        submenu->SetOnReturnToViewHandler(HANDLER(&PagerActionsScreen::onReturn));

        submenu->AddItem(
            resendToAllStr.format(CHIEF_COOKER_UI_TEXT("Resend %d (%s) to ALL", "重发 %d (%s) 给所有人"), actionValue, PagerActions::GetDescription(currentAction)),
            HANDLER_1ARG(&PagerActionsScreen::resendToAll)
        );

        if(currentAction == UNKNOWN) {
            submenu->AddItem(
                resendToCurrentStr.format(CHIEF_COOKER_UI_TEXT("Resend only to pager %d", "仅重发给呼机 %d"), pagerNum), HANDLER_1ARG(&PagerActionsScreen::resendSingle)
            );
        }

        actionsStrings = new String*[decoder->GetSupportedActionsCount()];
        for(size_t actionIndex = 0, i = 0; actionIndex < PagerActionCount; actionIndex++) {
            PagerAction action = static_cast<enum PagerAction>(actionIndex);
            if(!decoder->IsSupported(action)) {
                continue;
            }

            if(PagerActions::IsPagerActionSpecial(action)) {
                actionsStrings[i] = new String(CHIEF_COOKER_UI_TEXT("Trigger action %s", "触发动作 %s"), PagerActions::GetDescription(action));
            } else {
                actionsStrings[i] = new String(CHIEF_COOKER_UI_TEXT("%s only pager %d", "%s 仅呼机 %d"), PagerActions::GetDescription(action), pagerNum);
            }

            submenu->AddItem(actionsStrings[i]->cstr(), [this, action](uint32_t) { sendAction(action); });
            i++;
        }

        subghz->SetTransmitCompleteHandler(HANDLER(&PagerActionsScreen::txComplete));
    }

private:
    void resendToAll(uint32_t) {
        currentPager = 0;
        transmittingBatch = true;
        currentBatchFrequency = FrequencyManager::GetInstance()->GetFrequency(getPager()->frequency);

        batchTransmissionScreen = new BatchTransmissionScreen(config->MaxPagerForBatchOrDetection);
        UiManager::GetInstance()->PushView(batchTransmissionScreen->GetView());
        sendCurrentPager();

        FlipperDolphin::Deed(DolphinDeedSubGhzSend);
    }

    void resendSingle(uint32_t) {
        StoredPagerData* pager = getPager();
        uint32_t frequency = FrequencyManager::GetInstance()->GetFrequency(pager->frequency);
        subghz->Transmit(protocol->CreatePayload(pager->data, pager->te, config->SignalRepeats), frequency);

        FlipperDolphin::Deed(DolphinDeedSubGhzSend);
    }

    void sendAction(PagerAction action) {
        StoredPagerData* pager = getPager();
        uint32_t frequency = FrequencyManager::GetInstance()->GetFrequency(pager->frequency);
        subghz->Transmit(
            protocol->CreatePayload(decoder->SetAction(pager->data, action), pager->te, config->SignalRepeats), frequency
        );

        FlipperDolphin::Deed(DolphinDeedSubGhzSend);
    }

    void sendCurrentPager() {
        StoredPagerData* pager = getPager();
        batchTransmissionScreen->SetProgress(currentPager, config->MaxPagerForBatchOrDetection);
        subghz->Transmit(
            protocol->CreatePayload(decoder->SetPager(pager->data, currentPager), pager->te, config->SignalRepeats),
            currentBatchFrequency
        );
    }

    void txComplete() {
        if(transmittingBatch) {
            if(++currentPager <= config->MaxPagerForBatchOrDetection) {
                sendCurrentPager();
                return;
            } else {
                transmittingBatch = false;
                UiManager::GetInstance()->PopView(false);
            }
        }

        subghz->DefaultAfterTransmissionHandler();
    }

    void onReturn() {
        transmittingBatch = false;
    }

    void destroy() {
        subghz->SetTransmitCompleteHandler(NULL);
        for(size_t i = 0; i < decoder->GetSupportedActionsCount(); i++) {
            delete actionsStrings[i];
        }
        delete[] actionsStrings;
        delete this;
    }

public:
    UiView* GetView() {
        return submenu;
    }
};
