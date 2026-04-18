#include <furi.h>
#include <furi_hal.h>
#include "furi_hal_random.h"
#include <gui/elements.h>
#include <gui/gui.h>
#include <input/input.h>

#define TAG "Dice Roller"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define MULTI_DICE_UI_TEXT(en, zh) (zh)
#else
#define MULTI_DICE_UI_TEXT(en, zh) (en)
#endif

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* event_queue;
    DateTime datetime;
    uint8_t diceSelect;
    uint8_t diceQty;
    uint8_t diceRoll;
    uint8_t playerOneScore;
    uint8_t playerTwoScore;
    char rollTime[1][15];
    char diceType[1][11];
    char strings[5][45];
    char theScores[1][45];
    bool letsRoll;
} DiceState;

static void dice_input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_assert(event_queue);
    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void dice_render_callback(Canvas* const canvas, void* ctx) {
    DiceState* state = ctx;
    if(furi_mutex_acquire(state->mutex, 200) != FuriStatusOk) {
        // Can't obtain mutex, requeue render
        PluginEvent event = {.type = EventTypeTick};
        furi_message_queue_put(state->event_queue, &event, 0);
        return;
    }

    canvas_set_font(canvas, FontSecondary);
    if(state->diceSelect < 220) {
        if(state->diceQty == 1) {
            elements_button_left(canvas, "x1");
        } else if(state->diceQty == 2) {
            elements_button_left(canvas, "x2");
        } else if(state->diceQty == 3) {
            elements_button_left(canvas, "x3");
        } else if(state->diceQty == 4) {
            elements_button_left(canvas, "x4");
        } else if(state->diceQty == 5) {
            elements_button_left(canvas, "x5");
        } else if(state->diceQty == 6) {
            elements_button_left(canvas, "x6");
        }
    }
    if(state->letsRoll) {
        furi_hal_rtc_get_datetime(&state->datetime);
        uint8_t hour = state->datetime.hour;
        char strAMPM[3];
        snprintf(strAMPM, sizeof(strAMPM), "%s", "AM");
        if(hour > 12) {
            hour -= 12;
            snprintf(strAMPM, sizeof(strAMPM), "%s", "PM");
        }
        snprintf(
            state->rollTime[0],
            sizeof(state->rollTime[0]),
            "%.2d:%.2d:%.2d %s",
            hour,
            state->datetime.minute,
            state->datetime.second,
            strAMPM);
        if(state->diceSelect == 229) {
            const char* eightBall[] = {
                MULTI_DICE_UI_TEXT("It is certain", "当然"),
                MULTI_DICE_UI_TEXT("Without a doubt", "毫无疑问"),
                MULTI_DICE_UI_TEXT("You may rely on it", "可以相信"),
                MULTI_DICE_UI_TEXT("Yes definitely", "绝对是"),
                MULTI_DICE_UI_TEXT("It is decidedly so", "确定如此"),
                MULTI_DICE_UI_TEXT("As I see it, yes", "我看是的"),
                MULTI_DICE_UI_TEXT("Most likely", "很可能"),
                MULTI_DICE_UI_TEXT("Yes", "是"),
                MULTI_DICE_UI_TEXT("Outlook good", "前景不错"),
                MULTI_DICE_UI_TEXT("Signs point to yes", "迹象显示是"),
                MULTI_DICE_UI_TEXT("Reply hazy try again", "答案模糊，再试"),
                MULTI_DICE_UI_TEXT("Better not tell you now", "现在最好不说"),
                MULTI_DICE_UI_TEXT("Ask again later", "稍后再问"),
                MULTI_DICE_UI_TEXT("Cannot predict now", "现在无法预测"),
                MULTI_DICE_UI_TEXT("Concentrate and ask again", "专心后再问"),
                MULTI_DICE_UI_TEXT("Don't count on it", "别指望"),
                MULTI_DICE_UI_TEXT("Outlook not so good", "前景不太好"),
                MULTI_DICE_UI_TEXT("My sources say no", "消息源说不"),
                MULTI_DICE_UI_TEXT("Very doubtful", "很可疑"),
                MULTI_DICE_UI_TEXT("My reply is no", "我的回答是否")};
            state->diceRoll =
                ((rand() % state->diceSelect) + 1); // JUST TO GET IT GOING? AND FIX BUG
            snprintf(state->diceType[0], sizeof(state->diceType[0]), "%s", MULTI_DICE_UI_TEXT("8BALL", "魔法球"));
            snprintf(
                state->strings[0],
                sizeof(state->strings[0]),
                MULTI_DICE_UI_TEXT("%s at %s", "%s 于 %s"),
                state->diceType[0],
                state->rollTime[0]);
            uint8_t d1_i = rand() % COUNT_OF(eightBall);
            snprintf(state->strings[1], sizeof(state->strings[1]), "%s", eightBall[d1_i]);
        } else if(state->diceSelect == 228) {
            const char* eightBall[] = {
                MULTI_DICE_UI_TEXT("I'd do it.", "我会做"),
                MULTI_DICE_UI_TEXT("Hell, yeah!", "当然"),
                MULTI_DICE_UI_TEXT("You bet your life!", "赌上命都行"),
                MULTI_DICE_UI_TEXT("What are you waiting for?", "还等什么"),
                MULTI_DICE_UI_TEXT("You could do worse things.", "还能更糟"),
                MULTI_DICE_UI_TEXT("Sure, I won't tell.", "行，我不说"),
                MULTI_DICE_UI_TEXT("Yeah, you got this. Would I lie to you?", "能行，我会骗你吗"),
                MULTI_DICE_UI_TEXT("Looks like fun to me. ", "看起来挺有趣"),
                MULTI_DICE_UI_TEXT("Yeah, sure, why not?", "行啊，为何不"),
                MULTI_DICE_UI_TEXT("DO IT!!!", "去做!!!"),
                MULTI_DICE_UI_TEXT("Who's it gonna hurt?", "会伤到谁"),
                MULTI_DICE_UI_TEXT("Can you blame someone else?", "能甩锅吗"),
                MULTI_DICE_UI_TEXT("Ask me again later.", "稍后再问"),
                MULTI_DICE_UI_TEXT("Maybe, maybe not, I can't tell right now. ", "也许吧，现在说不准"),
                MULTI_DICE_UI_TEXT("Are you the betting type? ", "你敢赌吗"),
                MULTI_DICE_UI_TEXT("Don't blame me if you get caught.", "被抓别怪我"),
                MULTI_DICE_UI_TEXT("What have you got to lose?", "你还有啥可失去"),
                MULTI_DICE_UI_TEXT("I wouldn't if I were you.", "是我就不"),
                MULTI_DICE_UI_TEXT("My money's on the snowball.", "我押不行"),
                MULTI_DICE_UI_TEXT("Oh Hell no!", "绝对不!")};
            state->diceRoll =
                ((rand() % state->diceSelect) + 1); // JUST TO GET IT GOING? AND FIX BUG
            snprintf(state->diceType[0], sizeof(state->diceType[0]), "%s", MULTI_DICE_UI_TEXT("Devil Ball", "恶魔球"));
            snprintf(
                state->strings[0],
                sizeof(state->strings[0]),
                MULTI_DICE_UI_TEXT("%s at %s", "%s 于 %s"),
                state->diceType[0],
                state->rollTime[0]);
            uint8_t d1_i = rand() % COUNT_OF(eightBall);
            snprintf(state->strings[1], sizeof(state->strings[1]), "%s", eightBall[d1_i]);
        } else if(state->diceSelect == 231) {
            const char* deckOne[] = {"2H", "2C", "2D", "2S", "3H", "3C",  "3D",  "3S",  "4H",
                                     "4C", "4D", "4S", "5H", "5C", "5D",  "5S",  "6H",  "6C",
                                     "6D", "6S", "7H", "7C", "7D", "7S",  "8H",  "8C",  "8D",
                                     "8S", "9H", "9C", "9D", "9S", "10H", "10C", "10D", "10S",
                                     "JH", "JC", "JD", "JS", "QH", "QC",  "QD",  "QS",  "KH",
                                     "KC", "KD", "KS", "AH", "AC", "AD",  "AS"};
            char* deckTwo[] = {"2H", "2C", "2D", "2S", "3H", "3C",  "3D",  "3S",  "4H",
                               "4C", "4D", "4S", "5H", "5C", "5D",  "5S",  "6H",  "6C",
                               "6D", "6S", "7H", "7C", "7D", "7S",  "8H",  "8C",  "8D",
                               "8S", "9H", "9C", "9D", "9S", "10H", "10C", "10D", "10S",
                               "JH", "JC", "JD", "JS", "QH", "QC",  "QD",  "QS",  "KH",
                               "KC", "KD", "KS", "AH", "AC", "AD"}; // ONE LESS SINCE ONE WILL BE REMOVED
            state->diceRoll =
                ((rand() % state->diceSelect) + 1); // JUST TO GET IT GOING? AND FIX BUG
            snprintf(state->diceType[0], sizeof(state->diceType[0]), "%s", MULTI_DICE_UI_TEXT("WAR!", "战争!"));
            snprintf(
                state->strings[0],
                sizeof(state->strings[0]),
                MULTI_DICE_UI_TEXT("%s at %s", "%s 于 %s"),
                state->diceType[0],
                state->rollTime[0]);
            uint8_t d1_i = rand() % COUNT_OF(deckOne);
            // INITIALIZE WITH PLACEHOLDERS TO AVOID MAYBE UNINITIALIZED ERROR
            for(uint8_t i = 0; i < COUNT_OF(deckOne); i++) {
                if(i < d1_i) {
                    snprintf(deckTwo[i], 8, "%s", deckOne[i]);
                } else if(i > d1_i) {
                    snprintf(deckTwo[i - 1], 8, "%s", deckOne[i]);
                }
            }
            uint8_t d2_i = rand() % COUNT_OF(deckTwo);
            if(d1_i > d2_i) {
                state->playerOneScore++;
                snprintf(
                    state->strings[1],
                    sizeof(state->strings[1]),
                    "%s > %s",
                    deckOne[d1_i],
                    deckTwo[d2_i]);
            } else {
                state->playerTwoScore++;
                snprintf(
                    state->strings[1],
                    sizeof(state->strings[1]),
                    "%s < %s",
                    deckOne[d1_i],
                    deckTwo[d2_i]);
            }
        } else if(state->diceSelect == 232) {
            const char* diceOne[] = {
                MULTI_DICE_UI_TEXT("You", "你"),
                MULTI_DICE_UI_TEXT("You choose", "你来选"),
                MULTI_DICE_UI_TEXT("Nobody", "没人"),
                MULTI_DICE_UI_TEXT("Everyone", "所有人"),
                MULTI_DICE_UI_TEXT("Nose goes", "摸鼻者"),
                MULTI_DICE_UI_TEXT("Player to your right", "你右边玩家")};
            const char* diceTwo[] = {
                MULTI_DICE_UI_TEXT("take a tiny toke", "来一小口"),
                MULTI_DICE_UI_TEXT("just chill", "先冷静"),
                MULTI_DICE_UI_TEXT("take 2 tokes", "来两口"),
                MULTI_DICE_UI_TEXT("take a huge hit", "来一大口"),
                MULTI_DICE_UI_TEXT("bogart it", "别传了"),
                MULTI_DICE_UI_TEXT("take a puff", "吸一口")};
            const char* diceThree[] = {
                MULTI_DICE_UI_TEXT("while humming a tune", "一边哼歌"),
                MULTI_DICE_UI_TEXT("with your eyes closed", "闭着眼"),
                MULTI_DICE_UI_TEXT("on your knees", "跪着"),
                MULTI_DICE_UI_TEXT("while holding your nose", "捏着鼻子"),
                MULTI_DICE_UI_TEXT("while spinning in a circle", "一边转圈"),
                MULTI_DICE_UI_TEXT("in slow motion", "慢动作进行")};
            const char* diceFour[] = {
                MULTI_DICE_UI_TEXT("twice", "两次"),
                MULTI_DICE_UI_TEXT("then tell a joke", "然后讲笑话"),
                MULTI_DICE_UI_TEXT("then laugh as hard as you can", "然后大笑"),
                MULTI_DICE_UI_TEXT("with the player to your left", "和左边玩家一起"),
                MULTI_DICE_UI_TEXT("then sing a song", "然后唱首歌"),
                MULTI_DICE_UI_TEXT("then do a dance", "然后跳支舞")};
            state->diceRoll =
                ((rand() % state->diceSelect) + 1); // JUST TO GET IT GOING? AND FIX BUG
            snprintf(state->diceType[0], sizeof(state->diceType[0]), "%s", MULTI_DICE_UI_TEXT("WEED!", "烟局!"));
            snprintf(
                state->strings[0],
                sizeof(state->strings[0]),
                MULTI_DICE_UI_TEXT("%s at %s", "%s 于 %s"),
                state->diceType[0],
                state->rollTime[0]);
            uint8_t d1_i = rand() % COUNT_OF(diceOne);
            uint8_t d2_i = rand() % COUNT_OF(diceTwo);
            uint8_t d3_i = rand() % COUNT_OF(diceThree);
            uint8_t d4_i = rand() % COUNT_OF(diceFour);
            snprintf(state->strings[1], sizeof(state->strings[1]), "%s", diceOne[d1_i]);
            snprintf(state->strings[2], sizeof(state->strings[2]), "%s", diceTwo[d2_i]);
            snprintf(state->strings[3], sizeof(state->strings[3]), "%s", diceThree[d3_i]);
            snprintf(state->strings[4], sizeof(state->strings[4]), "%s", diceFour[d4_i]);
        } else {
            state->diceRoll = ((rand() % state->diceSelect) + 1);
            snprintf(
                state->diceType[0], sizeof(state->diceType[0]), "%s%d", "d", state->diceSelect);
            snprintf(
                state->strings[0],
                sizeof(state->strings[0]),
                MULTI_DICE_UI_TEXT("%d%s at %s", "%d%s 于 %s"),
                state->diceQty,
                state->diceType[0],
                state->rollTime[0]);
            if(state->diceQty == 1) {
                snprintf(state->strings[1], sizeof(state->strings[1]), "%d", state->diceRoll);
            } else if(state->diceQty == 2) {
                snprintf(
                    state->strings[1],
                    sizeof(state->strings[1]),
                    "%d %d",
                    state->diceRoll,
                    ((rand() % state->diceSelect) + 1));
            } else if(state->diceQty == 3) {
                snprintf(
                    state->strings[1],
                    sizeof(state->strings[1]),
                    "%d %d %d",
                    state->diceRoll,
                    ((rand() % state->diceSelect) + 1),
                    ((rand() % state->diceSelect) + 1));
            } else if(state->diceQty == 4) {
                snprintf(
                    state->strings[1],
                    sizeof(state->strings[1]),
                    "%d %d %d %d",
                    state->diceRoll,
                    ((rand() % state->diceSelect) + 1),
                    ((rand() % state->diceSelect) + 1),
                    ((rand() % state->diceSelect) + 1));
            } else if(state->diceQty == 5) {
                snprintf(
                    state->strings[1],
                    sizeof(state->strings[1]),
                    "%d %d %d %d %d",
                    state->diceRoll,
                    ((rand() % state->diceSelect) + 1),
                    ((rand() % state->diceSelect) + 1),
                    ((rand() % state->diceSelect) + 1),
                    ((rand() % state->diceSelect) + 1));
            } else if(state->diceQty == 6) {
                snprintf(
                    state->strings[1],
                    sizeof(state->strings[1]),
                    "%d %d %d %d %d %d",
                    state->diceRoll,
                    ((rand() % state->diceSelect) + 1),
                    ((rand() % state->diceSelect) + 1),
                    ((rand() % state->diceSelect) + 1),
                    ((rand() % state->diceSelect) + 1),
                    ((rand() % state->diceSelect) + 1));
            }
        }
        state->letsRoll = false;
    }
    furi_mutex_release(state->mutex);
    if(state->diceRoll != 0) {
        if(state->diceSelect == 232) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, state->strings[0]);
            canvas_draw_str_aligned(canvas, 64, 18, AlignCenter, AlignCenter, state->strings[1]);
            canvas_draw_str_aligned(canvas, 64, 26, AlignCenter, AlignCenter, state->strings[2]);
            canvas_draw_str_aligned(canvas, 64, 34, AlignCenter, AlignCenter, state->strings[3]);
            canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, state->strings[4]);
        } else if(state->diceSelect == 228 || state->diceSelect == 229) {
            canvas_set_font(canvas, FontBatteryPercent);
            canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignCenter, state->strings[1]);
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, state->strings[0]);
        } else {
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignCenter, state->strings[1]);
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, state->strings[0]);
        }
        if(state->diceSelect == 231 &&
           !(state->playerOneScore == 0 && state->playerTwoScore == 0)) {
            canvas_set_font(canvas, FontSecondary);
            snprintf(
                state->theScores[0],
                sizeof(state->theScores[0]),
                "%d                                   %d",
                state->playerOneScore,
                state->playerTwoScore);
            canvas_draw_str_aligned(canvas, 64, 34, AlignCenter, AlignCenter, state->theScores[0]);
        }
    }
    if(state->diceSelect == 229 || state->diceSelect == 228) {
        elements_button_center(canvas, MULTI_DICE_UI_TEXT("Shake", "摇一摇"));
    } else if(state->diceSelect == 231) {
        elements_button_center(canvas, MULTI_DICE_UI_TEXT("Draw", "抽牌"));
    } else {
        elements_button_center(canvas, MULTI_DICE_UI_TEXT("Roll", "掷骰"));
    }
    if(state->diceSelect == 2) {
        elements_button_right(canvas, "d2");
    } else if(state->diceSelect == 3) {
        elements_button_right(canvas, "d3");
    } else if(state->diceSelect == 4) {
        elements_button_right(canvas, "d4");
    } else if(state->diceSelect == 6) {
        elements_button_right(canvas, "d6");
    } else if(state->diceSelect == 8) {
        elements_button_right(canvas, "d8");
    } else if(state->diceSelect == 10) {
        elements_button_right(canvas, "d10");
    } else if(state->diceSelect == 12) {
        elements_button_right(canvas, "d12");
    } else if(state->diceSelect == 20) {
        elements_button_right(canvas, "d20");
    } else if(state->diceSelect == 59) {
        elements_button_right(canvas, "d59");
    } else if(state->diceSelect == 69) {
        elements_button_right(canvas, "d69");
    } else if(state->diceSelect == 100) {
        elements_button_right(canvas, "d100");
    } else if(state->diceSelect == 229) {
        elements_button_right(canvas, "8BALL");
    } else if(state->diceSelect == 228) {
        elements_button_right(canvas, "DBALL");
    } else if(state->diceSelect == 231) {
        elements_button_right(canvas, MULTI_DICE_UI_TEXT("WAR", "战争"));
    } else if(state->diceSelect == 232) {
        elements_button_right(canvas, MULTI_DICE_UI_TEXT("WEED", "烟局"));
    }
}

static void dice_state_init(DiceState* const state) {
    memset(state, 0, sizeof(DiceState));
    furi_hal_rtc_get_datetime(&state->datetime);
    state->diceSelect = 20;
    state->diceQty = 1;
    state->diceRoll = 0;
    state->playerOneScore = 0;
    state->playerTwoScore = 0;
    state->letsRoll = false;
}

static void dice_tick(void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    PluginEvent event = {.type = EventTypeTick};
    // It's OK to lose this event if system overloaded
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t dice_app(void* p) {
    UNUSED(p);
    DiceState* plugin_state = malloc(sizeof(DiceState));
    dice_state_init(plugin_state);
    plugin_state->event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));
    if(plugin_state->event_queue == NULL) {
        FURI_LOG_E(TAG, "cannot create event queue\n");
        free(plugin_state);
        return 255;
    }

    plugin_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(plugin_state->mutex == NULL) {
        FURI_LOG_E(TAG, "cannot create mutex\n");
        furi_message_queue_free(plugin_state->event_queue);
        free(plugin_state);
        return 255;
    }

    FuriTimer* timer =
        furi_timer_alloc(dice_tick, FuriTimerTypePeriodic, plugin_state->event_queue);
    if(timer == NULL) {
        FURI_LOG_E(TAG, "cannot create timer\n");
        furi_mutex_free(plugin_state->mutex);
        furi_message_queue_free(plugin_state->event_queue);
        free(plugin_state);
        return 255;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, dice_render_callback, plugin_state);
    view_port_input_callback_set(view_port, dice_input_callback, plugin_state->event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    furi_timer_start(timer, furi_kernel_get_tick_frequency());

    // Main loop
    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(plugin_state->event_queue, &event, 100);
        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypeShort || event.input.type == InputTypeRepeat) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        if(plugin_state->diceSelect == 2) {
                            plugin_state->diceSelect = 3;
                        } else if(plugin_state->diceSelect == 3) {
                            plugin_state->diceSelect = 4;
                        } else if(plugin_state->diceSelect == 4) {
                            plugin_state->diceSelect = 6;
                        } else if(plugin_state->diceSelect == 6) {
                            plugin_state->diceSelect = 8;
                        } else if(plugin_state->diceSelect == 8) {
                            plugin_state->diceSelect = 10;
                        } else if(plugin_state->diceSelect == 10) {
                            plugin_state->diceSelect = 12;
                        } else if(plugin_state->diceSelect == 12) {
                            plugin_state->diceSelect = 20;
                        } else if(plugin_state->diceSelect == 20) {
                            plugin_state->diceSelect = 100;
                        } else if(plugin_state->diceSelect == 100) {
                            plugin_state->playerOneScore = 0;
                            plugin_state->playerTwoScore = 0;
                            plugin_state->diceSelect = 231;
                        } else if(plugin_state->diceSelect == 231) {
                            plugin_state->diceSelect = 229;
                        } else if(plugin_state->diceSelect == 229) {
                            plugin_state->diceSelect = 228;
                        } else if(plugin_state->diceSelect == 228) {
                            plugin_state->diceSelect = 232;
                        } else if(plugin_state->diceSelect == 232) {
                            plugin_state->diceSelect = 59;
                        } else if(plugin_state->diceSelect == 59) {
                            plugin_state->diceSelect = 69;
                        } else {
                            plugin_state->diceSelect = 2;
                        }
                        break;
                    case InputKeyLeft:
                        if(plugin_state->diceQty <= 5) {
                            plugin_state->diceQty = plugin_state->diceQty + 1;
                        } else {
                            plugin_state->diceQty = 1;
                        }
                        break;
                    case InputKeyOk:
                        plugin_state->letsRoll = true;
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    default:
                        break;
                    }
                }
            } else if(event.type == EventTypeTick) {
                // furi_hal_rtc_get_datetime(&plugin_state->datetime);
            }
            view_port_update(view_port);
            furi_mutex_release(plugin_state->mutex);
        } else {
            // FURI_LOG_D(TAG, "osMessageQueue: event timeout");
        }
    }
    // Cleanup
    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(plugin_state->event_queue);
    furi_mutex_free(plugin_state->mutex);
    free(plugin_state);
    return 0;
}
