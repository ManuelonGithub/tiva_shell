


#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "stdio_hal/stdio_hal.h"
#include "shell.h"
#include "config/shell_cfg.h"
#include "shell_history.h"
#include "shell_readline.h"

/**************************************************************************************************/
/**************************************************************************************************/

const char* shell_line_prompt = ">> ";

static inline void print_whitespace(unsigned int n)
{
	const char spaces[] = "    ";
	if (n < 4) {
		stdio_put(spaces, n);
	}
	else {
		stdio_printf("\x1b[%dC", n);
	}
}

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
	ctx.cmd_size_max = strlen(ctx.shell_cmds[0].cmd_txt);

	// using insertion sort since the shell command count will be low
	// and odds are the linker has done a good job "pre-sorting" the array
	for (int i = 1; i < ctx.cmd_cnt; i++) {
		shell_cmd_t key = ctx.shell_cmds[i];
        int j = i - 1;

        while (j >= 0 && strcmp(ctx.shell_cmds[j].cmd_txt, key.cmd_txt) > 0) {
            ctx.shell_cmds[j + 1] = ctx.shell_cmds[j];
            j--;
        }

        ctx.shell_cmds[j + 1] = key;

		size_t cmd_len = strlen(key.cmd_txt);
		ctx.cmd_size_max = (ctx.cmd_size_max < cmd_len) ? cmd_len : ctx.cmd_size_max;
	}
}

static void shell_help()
{
	for (int i = 0; i < ctx.cmd_cnt; i++) {
		size_t cmd_len = stdio_print(ctx.shell_cmds[i].cmd_txt);
		print_whitespace(ctx.cmd_size_max-cmd_len);
		stdio_printf(" - %s\n", ctx.shell_cmds[i].help_txt);
	}
}

/**************************************************************************************************/
/**************************************************************************************************/

int find_command(const char* cmd)
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

void parse_and_run_cmd(char* cmd_text)
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

	int cmd_idx = find_command(cmd_text);
	if (cmd_idx < 0) {
		if (strcmp(cmd_text, "help") == 0) {
			shell_help();
		}
		else {
			stdio_printf("cmd \"%s\" not found\n", cmd_text);
		}
	}
	else {
		ctx.shell_cmds[cmd_idx].func(argc, argv);
	}
}

/**************************************************************************************************/
/**************************************************************************************************/

void shell_task()
{
	shell_ctx_init();
    // Prompt for text to be entered.
    stdio_print(
		"\n\n*********** Embedded Console ***********\n"
		"Enter \"help\" for the list of commands\n\n"
	);

    hist_init();

    // Loop forever echoing data through the UART.
    while(1) {
        char* cmd = shell_readline();
		
		// look into memchar
		while (*cmd == ' ') {
			cmd++;	// skip leading whitespace
		}
			
		if (*cmd != '\0') {
			hist_add_entry(cmd);
			parse_and_run_cmd(cmd);
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
		stdio_printf("\t(%d matches)\n", (last-first+1));
		for (size_t i = first; i <= last; i++) {
			size_t len = stdio_print(ctx.shell_cmds[i].cmd_txt);
			print_whitespace(ctx.cmd_size_max-len);
		}
	}
	else if (first >= 0) {
		*out_cmd = ctx.shell_cmds[first].cmd_txt;
	}

	return first;
}

/**************************************************************************************************/
/**************************************************************************************************/

static void history(int argc, char** argv)
{
	for (int i = hist_count()-1; i >= 0; i--) {
		stdio_printf("%u.\t%s\n", i, hist_cmd_by_offset(i));
	}
}

REGISTER_CMD("hist", history, "print currently saved history");
