#pragma once

#ifndef CHIEF_COOKER_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define CHIEF_COOKER_UI_TEXT(en, zh) (zh)
#else
#define CHIEF_COOKER_UI_TEXT(en, zh) (en)
#endif
#endif

#include "SelectCategoryScreen.hpp"
#include "app/AppConfig.hpp"
#include "app/pager/PagerReceiver.hpp"
#include "lib/String.hpp"
#include "lib/hardware/subghz/SubGhzModule.hpp"
#include "lib/ui/UiManager.hpp"
#include "lib/ui/view/VariableItemListUiView.hpp"

class SettingsScreen {
private:
    AppConfig* config;
    SubGhzModule* subghz;
    PagerReceiver* receiver;
    VariableItemListUiView* varItemList;

    UiVariableItem* currentCategoryItem;
    UiVariableItem* frequencyItem;
    UiVariableItem* maxPagerItem;
    UiVariableItem* signalRepeatItem;
    UiVariableItem* ignoreSavedItem;
    UiVariableItem* autosaveFoundItem;
    UiVariableItem* debugModeItem;

    String frequencyStr;
    String maxPagerStr;
    String signalRepeatStr;
    bool updateUserCategory;
    uint32_t categoryItemIndex;

public:
    SettingsScreen(AppConfig* config, PagerReceiver* receiver, SubGhzModule* subghz, bool updateUserCategory) {
        this->config = config;
        this->receiver = receiver;
        this->subghz = subghz;
        this->updateUserCategory = updateUserCategory;

        varItemList = new VariableItemListUiView();
        varItemList->SetOnDestroyHandler(HANDLER(&SettingsScreen::destroy));
        varItemList->SetEnterPressHandler(HANDLER_1ARG(&SettingsScreen::enterPressHandler));

        categoryItemIndex = varItemList->AddItem(
            currentCategoryItem = new UiVariableItem(CHIEF_COOKER_UI_TEXT("Category", "分类"), HANDLER_1ARG(&SettingsScreen::categoryChangedHandler))
        );

        varItemList->AddItem(
            frequencyItem = new UiVariableItem(
                CHIEF_COOKER_UI_TEXT("Scan frequency", "扫描频率"),
                FrequencyManager::GetInstance()->GetFrequencyIndex(config->Frequency),
                FrequencyManager::GetInstance()->GetFrequencyCount(),
                [this](uint8_t val) {
                    uint32_t freq = this->config->Frequency = FrequencyManager::GetInstance()->GetFrequency(val);
                    this->subghz->SetReceiveFrequency(this->config->Frequency);
                    return frequencyStr.format("%lu.%02lu", freq / 1000000, (freq % 1000000) / 10000);
                }
            )
        );

        varItemList->AddItem(
            maxPagerItem = new UiVariableItem(
                CHIEF_COOKER_UI_TEXT("Max pager value", "呼机最大值"),
                config->MaxPagerForBatchOrDetection - 1,
                UINT8_MAX,
                [this](uint8_t val) {
                    this->config->MaxPagerForBatchOrDetection = val + 1;
                    return maxPagerStr.fromInt(this->config->MaxPagerForBatchOrDetection);
                }
            )
        );

        varItemList->AddItem(
            signalRepeatItem = new UiVariableItem(
                CHIEF_COOKER_UI_TEXT("Times to repeat signal", "信号重复次数"),
                config->SignalRepeats - 1,
                UINT8_MAX,
                [this](uint8_t val) {
                    this->config->SignalRepeats = val + 1;
                    return signalRepeatStr.fromInt(this->config->SignalRepeats);
                }
            )
        );

        varItemList->AddItem(
            ignoreSavedItem = new UiVariableItem(
                CHIEF_COOKER_UI_TEXT("Saved stations", "已保存电台"),
                config->SavedStrategy,
                SavedStationStrategyValuesCount,
                [this](uint8_t val) {
                    this->config->SavedStrategy = static_cast<enum SavedStationStrategy>(val);
                    return savedStationsStrategy(this->config->SavedStrategy);
                }
            )
        );

        varItemList->AddItem(
            autosaveFoundItem = new UiVariableItem(
                CHIEF_COOKER_UI_TEXT("Autosave found signals", "自动保存发现的信号"),
                config->AutosaveFoundSignals,
                2,
                [this](uint8_t val) {
                    this->config->AutosaveFoundSignals = val;
                    return boolOption(val);
                }
            )
        );
    }

    UiView* GetView() {
        return varItemList;
    }

private:
    void enterPressHandler(uint32_t index) {
        if(index != categoryItemIndex) {
            return;
        }
        UiManager::GetInstance()->PushView(
            (new SelectCategoryScreen(false, User, HANDLER_2ARG(&SettingsScreen::categorySelected)))->GetView()
        );
    }

    void categorySelected(CategoryType, const char* category) {
        if(config->CurrentUserCategory != NULL) {
            delete config->CurrentUserCategory;
        }
        config->CurrentUserCategory = category != NULL ? new String("%s", category) : NULL;
        UiManager::GetInstance()->PopView(false);
        currentCategoryItem->Refresh();
    }

    const char* categoryChangedHandler(uint8_t) {
        const char* category = config->GetCurrentUserCategoryCstr();
        if(category == NULL) {
            category = CHIEF_COOKER_UI_TEXT("Default", "默认");
        }
        return category;
    }

    const char* boolOption(uint8_t value) {
        return value ? CHIEF_COOKER_UI_TEXT("ON", "开启") : CHIEF_COOKER_UI_TEXT("OFF", "关闭");
    }

    const char* savedStationsStrategy(SavedStationStrategy value) {
        switch(value) {
        case IGNORE:
            return CHIEF_COOKER_UI_TEXT("Ignore", "忽略");

        case SHOW_NAME:
            return CHIEF_COOKER_UI_TEXT("Show name", "显示名称");

        case HIDE:
            return CHIEF_COOKER_UI_TEXT("Hide", "隐藏");

        default:
            return NULL;
        }
    }

    void destroy() {
        config->Save();
        if(updateUserCategory) {
            receiver->SetUserCategory(config->CurrentUserCategory);
            receiver->ReloadKnownStations();
        }

        delete currentCategoryItem;
        delete frequencyItem;
        delete maxPagerItem;
        delete signalRepeatItem;
        delete ignoreSavedItem;
        delete autosaveFoundItem;
        delete debugModeItem;

        delete this;
    }
};
