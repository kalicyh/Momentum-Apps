#include "../minesweeper.h"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define MS_UI_TEXT(en, zh) (zh)
#else
#define MS_UI_TEXT(en, zh) (en)
#endif

static const char* farewellPrompts[] = {
    MS_UI_TEXT("Leaving so\n   soon?", "这么快就要\n   走吗?"),
    MS_UI_TEXT("Will you\n   stay?", "还要继续\n   吗?"),
    MS_UI_TEXT("Don't swim\n   away.", "别这么快\n   游走。"),
    MS_UI_TEXT("Feeling\n   farewell?", "真的要\n   离开吗?"),
    MS_UI_TEXT("Sad to part\n   ways.", "真舍不得\n   你走。"),
    MS_UI_TEXT("Ocean of\n   goodbyes.", "满是离别的\n   海洋。"),
    MS_UI_TEXT("Dolphin tears,\n   why?", "海豚落泪,\n   为什么?"),
    MS_UI_TEXT("Lonely without\n   you.", "没有你会\n   很孤单。"),
    MS_UI_TEXT("End of our\n   wave?", "我们的浪花\n   到此为止?"),
    MS_UI_TEXT("Will you\n   return?", "你还会\n   回来吗?"),
    MS_UI_TEXT("Drowning in\n   goodbye.", "沉没在\n   告别里。"),
    MS_UI_TEXT("Farewell\n   ripples.", "告别的\n   涟漪。"),
    MS_UI_TEXT("Flipper's\n   frown.", "Flipper 的\n   失落。"),
    MS_UI_TEXT("Sea of\n   solitude.", "孤独的\n   海。"),
    MS_UI_TEXT("Parting\n   currents.", "离别的\n   洋流。"),
    MS_UI_TEXT("Goodbye, old\n   friend.", "再见了,\n   老朋友。"),
    MS_UI_TEXT("Will tide\n  bring back?", "潮汐还会\n  带你回来吗?"),
    MS_UI_TEXT("Echoes of\n  departure.", "离去的\n  回声。"),
    MS_UI_TEXT("Dolphin's last\n   dance.", "海豚最后的\n   舞步。"),
    MS_UI_TEXT("Ocean misses\n   you.", "海洋会想念\n   你。"),
};

static void minesweeper_menu_scene_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);

    MineSweeperApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void minesweeper_scene_menu_screen_on_enter(void* context) {
    furi_assert(context);
    MineSweeperApp* app = (MineSweeperApp*)context;

    view_dispatcher_switch_to_view(app->view_dispatcher, MineSweeperLoadingView);

    dialog_ex_set_context(app->menu_screen, app);

    dialog_ex_set_header(
        app->menu_screen, MS_UI_TEXT("Exit Game?", "退出游戏?"), (128 * 3) / 4, 4, AlignCenter, AlignTop);

    uint32_t prompt_index = furi_hal_random_get() % sizeof(farewellPrompts) / sizeof(char*);

    dialog_ex_set_text(
        app->menu_screen, farewellPrompts[prompt_index], 65, 20, AlignLeft, AlignTop);

    dialog_ex_set_icon(app->menu_screen, 11, 0, &I_dolph_cry_49x54);

    dialog_ex_set_left_button_text(app->menu_screen, MS_UI_TEXT("Back", "返回"));
    dialog_ex_set_center_button_text(app->menu_screen, MS_UI_TEXT("Settings", "设置"));
    dialog_ex_set_right_button_text(app->menu_screen, MS_UI_TEXT("Exit", "退出"));

    dialog_ex_set_result_callback(app->menu_screen, minesweeper_menu_scene_dialog_callback);

    view_dispatcher_switch_to_view(app->view_dispatcher, MineSweeperMenuView);
}

bool minesweeper_scene_menu_screen_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    MineSweeperApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultLeft:
            if(!scene_manager_search_and_switch_to_previous_scene(
                   app->scene_manager, MineSweeperSceneGameScreen)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }

            consumed = true;
            break;

        case DialogExResultRight:
            scene_manager_stop(app->scene_manager);
            view_dispatcher_stop(app->view_dispatcher);
            consumed = true;
            break;

        case DialogExResultCenter:
            scene_manager_next_scene(app->scene_manager, MineSweeperSceneSettingsScreen);
            break;

        default:
            break;
        }
    }

    return consumed;
}

void minesweeper_scene_menu_screen_on_exit(void* context) {
    furi_assert(context);
    MineSweeperApp* app = (MineSweeperApp*)context;

    dialog_ex_reset(app->menu_screen);
}
