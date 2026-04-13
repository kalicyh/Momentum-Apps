/* 
 * This file is part of the INA Meter application for Flipper Zero (https://github.com/cepetr/flipper-tina).
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "app.h"
#include "scenes/scenes.h"

static void dialog_callback(DialogExResult result, void* context) {
    App* app = (App*)context;

    if(result == DialogExResultRight) {
        scene_manager_stop(app->scene_manager);
        view_dispatcher_stop(app->view_dispatcher);
    } else {
        scene_manager_next_scene(app->scene_manager, SceneGauge);
    }
}

void scene_dialog_exit_on_enter(void* context) {
    App* app = (App*)context;

    const char* title = INA_METER_UI_TEXT("Exit application?", "退出应用?");
    const char* message;

    if(app->datalog != NULL) {
        message = INA_METER_UI_TEXT("Recording is in progress. Are you sure?", "正在录制中，确定退出?");
    } else {
        message = INA_METER_UI_TEXT("Are you sure?", "确定吗?");
    }

    dialog_ex_set_header(app->dialog, title, 64, 0, AlignCenter, AlignTop);
    dialog_ex_set_text(app->dialog, message, 64, 24, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(app->dialog, INA_METER_UI_TEXT("No", "否"));
    dialog_ex_set_right_button_text(app->dialog, INA_METER_UI_TEXT("Yes", "是"));

    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_result_callback(app->dialog, dialog_callback);

    view_dispatcher_switch_to_view(app->view_dispatcher, AppViewDialog);
}

bool scene_dialog_exit_on_event(void* context, SceneManagerEvent event) {
    bool consumed = false;

    UNUSED(context);
    UNUSED(event);

    return consumed;
}

void scene_dialog_exit_on_exit(void* context) {
    App* app = (App*)context;

    dialog_ex_reset(app->dialog);
}
