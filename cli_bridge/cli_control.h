#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <toolbox/pipe.h>
extern void clicontrol_hijack(size_t tx_size, size_t rx_size);
extern void clicontrol_unhijack(bool persist);
extern PipeSide* cli_pipe;
