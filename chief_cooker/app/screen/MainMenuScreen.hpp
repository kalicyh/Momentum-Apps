#pragma once

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define CHIEF_COOKER_UI_TEXT(en, zh) (zh)
#else
#define CHIEF_COOKER_UI_TEXT(en, zh) (en)
#endif

#include "app/AppConfig.hpp"

#include "lib/ui/view/UiView.hpp"
#include "lib/ui/view/SubMenuUiView.hpp"
#include "lib/ui/UiManager.hpp"

#include "app/AppNotifications.hpp"

#include "SelectCategoryScreen.hpp"
#include "ScanStationsScreen.hpp"

class MainMenuScreen {
private:
    AppConfig* config;
    SubMenuUiView* menuView;

public:
    MainMenuScreen(AppConfig* config) {
        this->config = config;

        menuView = new SubMenuUiView("Chief Cooker");
        menuView->AddItem(CHIEF_COOKER_UI_TEXT("Scan for station signals", "扫描电台信号"), HANDLER_1ARG(&MainMenuScreen::scanStationsMenuPressed));
        menuView->AddItem(CHIEF_COOKER_UI_TEXT("Saved stations database", "已保存电台数据库"), HANDLER_1ARG(&MainMenuScreen::stationDatabasePressed));
        menuView->AddItem(CHIEF_COOKER_UI_TEXT("About / Manual", "关于 / 说明"), HANDLER_1ARG(&MainMenuScreen::aboutPressed));
        menuView->SetOnDestroyHandler(HANDLER(&MainMenuScreen::destroy));
    }

    UiView* GetView() {
        return menuView;
    }

private:
    void scanStationsMenuPressed(uint32_t) {
        UiManager::GetInstance()->ShowLoading();
        UiManager::GetInstance()->PushView((new ScanStationsScreen(config))->GetView());
    }

    void stationDatabasePressed(uint32_t) {
        SubMenuUiView* savedMenuView = new SubMenuUiView(CHIEF_COOKER_UI_TEXT("Select database", "选择数据库"));
        savedMenuView->AddItem(CHIEF_COOKER_UI_TEXT("Saved by you", "手动保存"), HANDLER_1ARG(&MainMenuScreen::savedStationsPressed));
        savedMenuView->AddItem(CHIEF_COOKER_UI_TEXT("Autosaved", "自动保存"), HANDLER_1ARG(&MainMenuScreen::autosavedStationsPressed));
        UiManager::GetInstance()->PushView(savedMenuView);
    }

    void savedStationsPressed(uint32_t) {
        UiManager::GetInstance()->ShowLoading();
        UiManager::GetInstance()->PushView(
            (new SelectCategoryScreen(false, User, HANDLER_2ARG(&MainMenuScreen::categorySelected)))->GetView()
        );
    }

    void autosavedStationsPressed(uint32_t) {
        UiManager::GetInstance()->ShowLoading();
        UiManager::GetInstance()->PushView(
            (new SelectCategoryScreen(false, Autosaved, HANDLER_2ARG(&MainMenuScreen::categorySelected)))->GetView()
        );
    }

    void categorySelected(CategoryType categoryType, const char* category) {
        UiManager::GetInstance()->ShowLoading();
        UiManager::GetInstance()->PushView((new ScanStationsScreen(config, categoryType, category))->GetView());
    }

    void aboutPressed(uint32_t index) {
        UNUSED(index);

        Notification::Play(&NOTIFICATION_PAGER_RECEIVE);
        menuView->SetItemLabel(index, CHIEF_COOKER_UI_TEXT("Developed by Denr01!", "由 Denr01 开发！"));
    }

    void destroy() {
        delete this;
    }
};
