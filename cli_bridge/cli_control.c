#include "cli_control.h"

#include <cli/cli.h>
#include <toolbox/cli/shell/cli_shell.h>

static PipeSide* cli_shell_pipe = NULL;
static CliShell* cli_shell = NULL;
PipeSide* cli_pipe = NULL;

static void clicontrol_motd(void* context) {
    UNUSED(context);
    printf("CLI Bridge\r\nRun `help` or `?` to list available commands\r\n\r\n");
}

static const CliCommandExternalConfig clicontrol_ext_config = {
    .search_directory = "/ext/apps_data/cli/plugins",
    .fal_prefix = "cli_",
    .appid = "cli",
};

void clicontrol_hijack(size_t tx_size, size_t rx_size) {
    if(cli_pipe) return;

    CliRegistry* registry = furi_record_open(RECORD_CLI);

    PipeSideReceiveSettings app_settings = {
        .capacity = tx_size,
        .trigger_level = 1,
    };
    PipeSideReceiveSettings shell_settings = {
        .capacity = rx_size,
        .trigger_level = 1,
    };
    PipeSideBundle bundle = pipe_alloc_ex(app_settings, shell_settings);
    cli_pipe = bundle.alices_side;
    cli_shell_pipe = bundle.bobs_side;

    cli_shell =
        cli_shell_alloc(clicontrol_motd, NULL, cli_shell_pipe, registry, &clicontrol_ext_config);
    cli_shell_start(cli_shell);

    furi_record_close(RECORD_CLI);
}

void clicontrol_unhijack(bool persist) {
    if(!cli_pipe) return;
    if(persist) return;

    static const char exit_cmd[] = "exit\r\n";
    pipe_send(cli_pipe, exit_cmd, sizeof(exit_cmd) - 1);

    cli_shell_join(cli_shell);
    cli_shell_free(cli_shell);
    cli_shell = NULL;

    pipe_free(cli_shell_pipe);
    pipe_free(cli_pipe);
    cli_shell_pipe = NULL;
    cli_pipe = NULL;
}
