#include <stdbool.h>
#include <stdint.h>
#include "flizzer_tracker.h"

char* audio_modes_text[2] = {
    FLIZZER_TRACKER_UI_TEXT("Internal", "内置"),
    FLIZZER_TRACKER_UI_TEXT("External", "外置"),
};
bool audio_modes_values[2] = {false, true};