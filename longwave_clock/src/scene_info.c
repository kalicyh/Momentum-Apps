#include "scene_info.h"
#include "longwave_clock_app.h"
#include "app_state.h"
#include "scenes.h"

void lwc_info_scene_on_enter(void* context) {
    App* app = context;

    text_box_set_text(
        app->info_text,
        LWC_UI_TEXT(
            "Long wave time signal senders broadcast the exact time and date over LW radio. "
            "If you're in range, you can receive their signals with an inexpensive receiver tuned to their frequency. "
            "These come with a big enough ferrite core, and some low pass filtering. "
            "Keep the ferrite core away from the flipper (3-5cm) and away from E/M radiation (at least 1 metre away from a monitor). "
            "Best reception next to a window or outside with no obstructions. Nights are best. "
            "Keep the ferrite core level to ground and  perpendicular to the sender if possible.",
            "长波时间信号发射台通过长波无线电广播精确的时间和日期。"
            "如果在范围内，可以使用调谐到其频率的廉价接收器接收信号。"
            "接收器配有足够大的铁氧体磁芯和低通滤波器。"
            "将铁氧体磁芯远离 Flipper（3-5厘米），远离电磁辐射（至少距显示器1米）。"
            "最佳接收位置在窗边或室外无遮挡处，夜间效果最佳。"
            "尽量保持铁氧体磁芯水平于地面且垂直于发射台方向。"));

    view_dispatcher_switch_to_view(app->view_dispatcher, LWCInfoView);
}

bool lwc_info_scene_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);

    return false;
}

void lwc_info_scene_on_exit(void* context) {
    App* app = context;

    text_box_reset(app->about);
}
