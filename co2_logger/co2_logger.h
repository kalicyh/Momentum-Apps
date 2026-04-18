#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define CO2_LOGGER_UI_TEXT(en, zh) (zh)
#else
#define CO2_LOGGER_UI_TEXT(en, zh) (en)
#endif

typedef struct co2_logger co2_logger;

co2_logger* co2_logger_alloc();

void co2_logger_free(co2_logger* instance);

void co2_logger_open(co2_logger* instance);

void co2_logger_close(co2_logger* instance);

bool co2_logger_read_gas_concentration(co2_logger* instance, uint32_t* value);
