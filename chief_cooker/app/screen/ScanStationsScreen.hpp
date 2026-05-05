#pragma once

#ifndef CHIEF_COOKER_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define CHIEF_COOKER_UI_TEXT(en, zh) (zh)
#else
#define CHIEF_COOKER_UI_TEXT(en, zh) (en)
#endif
#endif

#include "SettingsScreen.hpp"
#include "lib/hardware/subghz/data/SubGhzReceivedDataStub.hpp"
#include "lib/ui/view/ColumnOrientedListUiView.hpp"

#include "EditPagerScreen.hpp"
#include "PagerActionsScreen.hpp"

#include "lib/hardware/subghz/SubGhzModule.hpp"

#include "app/AppConfig.hpp"
#include "app/AppNotifications.hpp"
#include "app/pager/PagerReceiver.hpp"

static int8_t stationScreenColumnOffsets[]{
    3, // station name (if known)
    3, // hex
    49, // station
    72, // pager
    94, // action
    128 - 8 // repeats / edit flag
};
static Font stationScreenColumnFonts[]{
    FontSecondary, // station name (if known)
    FontBatteryPercent, // hex
    FontSecondary, // station
    FontSecondary, // pager
    FontBatteryPercent, // action
    FontBatteryPercent, // repeats
};

static Align stationScreenColumnAlignments[]{
    AlignLeft, // station name (if known)
    AlignLeft, // hex
    AlignCenter, // station
    AlignCenter, // pager
    AlignCenter, // action
    AlignRight, // repeats
};

class ScanStationsScreen {
private:
    AppConfig* config;
    ColumnOrientedListUiView* menuView;
    PagerReceiver* pagerReceiver;
    SubGhzModule* subghz;
    bool receiveMode = false;
    bool updateUserCategory = true;
    int scanForMoreButtonIndex = -1;
    uint32_t fromFilePagersCount = 0;

public:
    ScanStationsScreen(AppConfig* config) : ScanStationsScreen(config, true, NotSelected, NULL) {
    }

    ScanStationsScreen(AppConfig* config, CategoryType categoryType, const char* category) :
            ScanStationsScreen(config, false, categoryType, category) {
    }

    ScanStationsScreen(AppConfig* config, bool receiveNew, CategoryType categoryType, const char* category) {
        this->config = config;

        menuView = new ColumnOrientedListUiView(
            stationScreenColumnOffsets,
            sizeof(stationScreenColumnOffsets),
            HANDLER_3ARG(&ScanStationsScreen::getElementColumnName)
        );
        menuView->SetOnDestroyHandler(HANDLER(&ScanStationsScreen::destroy));
        menuView->SetOnReturnToViewHandler([this]() { this->menuView->Refresh(); });
        menuView->SetGoBackHandler(HANDLER(&ScanStationsScreen::goBack));

        menuView->SetColumnFonts(stationScreenColumnFonts);
        menuView->SetColumnAlignments(stationScreenColumnAlignments);

        menuView->SetLeftButton(CHIEF_COOKER_UI_TEXT("Conf", "设置"), HANDLER_1ARG(&ScanStationsScreen::showConfig));

        subghz = new SubGhzModule(config->Frequency);
        subghz->SetReceiveHandler(HANDLER_1ARG(&ScanStationsScreen::receive));
        if(receiveNew) {
            subghz->SetReceiveAfterTransmission(true);
            subghz->ReceiveAsync();
        }

        pagerReceiver = new PagerReceiver(config);
        if(categoryType == User) {
            pagerReceiver->SetUserCategory(category);
            updateUserCategory = false;

            if(category != NULL) {
                menuView->SetRightButton(CHIEF_COOKER_UI_TEXT("Delete category", "删除分类"), HANDLER_1ARG(&ScanStationsScreen::deleteCategory));
            }
        } else {
            pagerReceiver->ReloadKnownStations();
        }

        if(receiveNew) {
            if(subghz->IsExternal()) {
                menuView->SetNoElementCaption(CHIEF_COOKER_UI_TEXT("Receiving via EXT...", "通过外接接收中..."));
            } else {
                menuView->SetNoElementCaption(CHIEF_COOKER_UI_TEXT("Receiving...", "接收中..."));
            }
        } else {
            menuView->SetNoElementCaption(CHIEF_COOKER_UI_TEXT("No stations found!", "未找到电台！"));
        }

        if(!receiveNew) {
            pagerReceiver->LoadStationsFromDirectory(categoryType, category, HANDLER_1ARG(&ScanStationsScreen::pagerAdded));

            if(categoryType == User && menuView->GetElementsCount() > 0) {
                scanForMoreButtonIndex = menuView->GetElementsCount();
                fromFilePagersCount = menuView->GetElementsCount();
                menuView->AddElement();
            }
        }

        receiveMode = receiveNew;
    }

    UiView* GetView() {
        return menuView;
    }

private:
    void receive(SubGhzReceivedData* data) {
        pagerAdded(pagerReceiver->Receive(data));
        delete data;
    }

    void pagerAdded(ReceivedPagerData* pagerData) {
        if(pagerData != NULL) {
            if(pagerData->IsNew()) {
                if(receiveMode) {
                    Notification::Play(&NOTIFICATION_PAGER_RECEIVE);
                }

                if(pagerData->GetIndex() == 0) { // add buttons after capturing the first transmission
                    menuView->SetCenterButton(CHIEF_COOKER_UI_TEXT("Actions", "操作"), HANDLER_1ARG(&ScanStationsScreen::showActions));
                    menuView->SetRightButton(CHIEF_COOKER_UI_TEXT("Edit", "编辑"), HANDLER_1ARG(&ScanStationsScreen::editPagerMessage));
                }

                if(!receiveMode || scanForMoreButtonIndex == -1) {
                    menuView->AddElement();
                } else {
                    scanForMoreButtonIndex = -1;
                }
            }

            if(menuView->IsOnTop()) {
                menuView->Refresh();
            }

            delete pagerData;
        }
    }

    void getElementColumnName(int index, int column, String* str) {
        StoredPagerData* pagerData = pagerReceiver->PagerGetter(index)();
        PagerDecoder* decoder = pagerReceiver->decoders[pagerData->decoder];
        String* name = pagerReceiver->GetName(pagerData);

        if(index == scanForMoreButtonIndex) {
            if(column == 0) {
                if(!receiveMode) {
                    str->format(CHIEF_COOKER_UI_TEXT("> Scan here for more", "> 点此扫描更多"));
                } else {
                    str->format(CHIEF_COOKER_UI_TEXT("Scanning...", "扫描中..."));
                }
            }
            return;
        }

        switch(column) {
        case 0: // station name
            if(name != NULL) {
                str->format("%s", name->cstr());
            }
            break;

        case 1: // hex
            if(name == NULL) {
                str->format("%06X", pagerData->data);
            }
            break;

        case 2: // station
            if(name == NULL) {
                str->format("%d", decoder->GetStation(pagerData->data));
            }
            break;

        case 3: // pager
            str->format("%d", decoder->GetPager(pagerData->data));
            break;

        case 4: // action
        {
            PagerAction action = decoder->GetAction(pagerData->data);
            if(action == UNKNOWN) {
                str->format("%d", decoder->GetActionValue(pagerData->data));
            } else {
                str->format("%.4s", PagerActions::GetDescription(action));
            }
        }; break;

        case 5: // repeats or edit flag
            if(pagerData->edited) {
                str->format("**");
            } else if(receiveMode) {
                str->format("x%d", pagerData->repeats);
            }
            break;

        default:
            break;
        }
    }

    void showConfig(uint32_t) {
        SettingsScreen* screen = new SettingsScreen(config, pagerReceiver, subghz, updateUserCategory);
        UiManager::GetInstance()->PushView(screen->GetView());
    }

    void editPagerMessage(uint32_t index) {
        if((int)index == scanForMoreButtonIndex) {
            return;
        }

        PagerDataGetter getPager = pagerReceiver->PagerGetter(index);
        EditPagerScreen* screen = new EditPagerScreen(config, subghz, pagerReceiver, getPager, index < fromFilePagersCount);
        UiManager::GetInstance()->PushView(screen->GetView());
    }

    void showActions(uint32_t index) {
        if((int)index == scanForMoreButtonIndex) {
            if(!receiveMode) {
                subghz->SetReceiveAfterTransmission(true);
                subghz->ReceiveAsync();

                receiveMode = true;
            }
            return;
        }

        PagerDataGetter getPager = pagerReceiver->PagerGetter(index);
        StoredPagerData* pagerData = getPager();
        PagerDecoder* decoder = pagerReceiver->decoders[pagerData->decoder];
        PagerProtocol* protocol = pagerReceiver->protocols[pagerData->protocol];

        PagerActionsScreen* screen = new PagerActionsScreen(config, getPager, decoder, protocol, subghz);
        UiManager::GetInstance()->PushView(screen->GetView());
    }

    bool goBack() {
        if(receiveMode && menuView->GetElementsCount() > 0) {
            DialogUiView* confirmGoBack = new DialogUiView(CHIEF_COOKER_UI_TEXT("Really stop scan?", "确认停止扫描?"), CHIEF_COOKER_UI_TEXT("You may loose captured signals", "可能会丢失已捕获的信号"));
            confirmGoBack->AddLeftButton(CHIEF_COOKER_UI_TEXT("No", "否"));
            confirmGoBack->AddRightButton(CHIEF_COOKER_UI_TEXT("Yes", "是"));
            confirmGoBack->SetResultHandler(HANDLER_1ARG(&ScanStationsScreen::goBackConfirmationHandler));
            UiManager::GetInstance()->PushView(confirmGoBack);
            return false;
        }
        return true;
    }

    void deleteCategory(int) {
        AppFileSysytem().DeleteCategory(pagerReceiver->GetCurrentUserCategory());
        menuView->SetRightButton(CHIEF_COOKER_UI_TEXT("Deleted", "已删除"), NULL);
    }

    void goBackConfirmationHandler(DialogExResult dialogResult) {
        if(dialogResult == DialogExResultRight) {
            UiManager::GetInstance()->PopView(false);
        }
    }

    void destroy() {
        delete subghz;
        delete pagerReceiver;
        delete this;
    }
};
