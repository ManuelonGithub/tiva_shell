


#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "stdio_hal/stdio_hal.h"
#include "shell.h"
#include "config/shell_cfg.h"
#include "shell_history.h"
#include "readline.h"

/**************************************************************************************************/
/**************************************************************************************************/

const char* shell_line_prompt = ">> ";

typedef struct {
	shell_cmd_t* shell_cmds;
	size_t cmd_cnt;

	size_t cmd_size_max;
} shell_ctx;

static shell_ctx ctx;

void shell_ctx_init()
{
	extern uint32_t _shell_cmds, _eshell_cmds;

    ctx.cmd_cnt = ((unsigned long)&_eshell_cmds - (unsigned long)&_shell_cmds)/sizeof(shell_cmd_t);
    ctx.shell_cmds = (shell_cmd_t*)&_shell_cmds;

	ctx.cmd_size_max = 0;

	int j;
	shell_cmd_t key; 

	for (int i = 1; i < ctx.cmd_cnt; i++) {
		key = ctx.shell_cmds[i];
        j = i - 1;

        while (j >= 0 && strcmp(ctx.shell_cmds[j].cmd_txt, key.cmd_txt) > 0) {
            ctx.shell_cmds[j + 1] = ctx.shell_cmds[j];
            j = j - 1;
        }

        ctx.shell_cmds[j + 1] = key;
	}

	stdio_printf("Registered %d shell commands in address %x: \n", ctx.cmd_cnt, ctx.shell_cmds);

	for (int i = 0; i < ctx.cmd_cnt; i++) {
		stdio_printf("%s - %s\n", ctx.shell_cmds[i].cmd_txt, ctx.shell_cmds[i].help_txt);
	}
}

/**************************************************************************************************/
/**************************************************************************************************/

size_t find_command(const char* cmd)
{	
	size_t ret = SIZE_MAX;

	for (size_t idx = 0; idx < ctx.cmd_cnt; idx++) {
		int cmp = strcmp(cmd, ctx.shell_cmds[idx].cmd_txt);

		if (cmp == 0) {
			ret = idx;
			break;
		}
		else if (cmp < 0) {
			break;
		}
	}

	return ret;
}

void parse_and_run_cmd(char* cmd_text, size_t hint_idx)
{
	char* argv[8];
	int argc = 0;

	char* traveller = cmd_text;

	while (*traveller != '\0' && argc < 8) {
		while (*(++traveller) > ' ') {}

		if (*traveller == ' ') {
			*traveller = '\0';
			while (*(++traveller) == ' ') {}	// look into memchar
			
			if (*traveller > ' ') {
				argv[argc++] = traveller;
			}
		}
		else {
			*traveller = '\0';
		}
	}

	if ((hint_idx < ctx.cmd_cnt)) {
		if (strcmp(cmd_text, ctx.shell_cmds[hint_idx].cmd_txt) == 0) {
			ctx.shell_cmds[hint_idx].func(argc, argv);
		}
		else {
			size_t cmd_idx = find_command(cmd_text);
			if (cmd_idx < ctx.cmd_cnt) {
				ctx.shell_cmds[cmd_idx].func(argc, argv);
			}
			else {
				stdio_printf("cmd \"%s\" not found\n", cmd_text);
			}
		}
	}
	else {
		stdio_printf("cmd \"%s\" not found\n", cmd_text);
	}
}

/**************************************************************************************************/
/**************************************************************************************************/

void shell_task()
{
	shell_ctx_init();
    // Prompt for text to be entered.
    stdio_print("\n\n\n\n=~=~=~=~=~ UART SHELL ~=~=~=~=~=\n");

    shell_hist_init();

    // Loop forever echoing data through the UART.
    while(1) {
        char* cmd = shell_readline();
		
		// look into memchar
		while (*cmd == ' ') {
			cmd++;	// skip leading whitespace
		}
			
		if (*cmd != '\0') {
			add_shell_cmd(cmd);
			parse_and_run_cmd(cmd, 0);
		}
    }
}

/**************************************************************************************************/
/**************************************************************************************************/

size_t find_or_print_matches(const char* cmd_text, size_t len, const char** out_cmd)
{
	int first = -1, last = -1;
	*out_cmd = NULL;

	size_t i = 0;
	for (; i < ctx.cmd_cnt; i++) {
		int res = strncmp(cmd_text, ctx.shell_cmds[i].cmd_txt, len);
        if (res == 0) {
			first = i++;
			break;
        }
		if (res < 0) {
			i = ctx.cmd_cnt;
		}
    }

	for (; i < ctx.cmd_cnt; i++) {
		int res = strncmp(cmd_text, ctx.shell_cmds[i].cmd_txt, len);
        if (res == 0) {
			last = i;
        }
		if (res < 0) {
			break;
		}
    }

	if (last > 0) {
		stdio_printf("\n%d matches: %s", last, ctx.shell_cmds[first].cmd_txt);
		for (size_t i = first; i < last; i++) {
			stdio_printf("\t%s", ctx.shell_cmds[i].cmd_txt);
			if (i & 4) {	// limit columns to 4
				stdio_print("\n");
			}
		}
	}
	else if (first >= 0) {
		*out_cmd = ctx.shell_cmds[first].cmd_txt;
	}

	return first;
}
