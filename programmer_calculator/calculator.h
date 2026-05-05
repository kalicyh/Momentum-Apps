#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "calculator_state.h"
#include "input_handling.h"
#include "calculation_logic.h"
#include "ui_drawing.h"
#include "utilities.h"
#include "init_cleanup.h"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define PROGRAMMER_CALC_UI_TEXT(en, zh) (zh)
#else
#define PROGRAMMER_CALC_UI_TEXT(en, zh) (en)
#endif

#endif // CALCULATOR_H
