


#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "stdio_hal/stdio_hal.h"
#include "shell.h"
#include "shell_cfg.h"
#include "shell_history.h"


#define CMD_BUF_LEN CMD_MAX_LEN+1

const char* cmd_prompt = "\n>> ";

static void cursor_insert_char(char c);
static void cursor_backspace();
static void cursor_delete();
static void cursor_lmove(unsigned int cnt);
static void cursor_rmove(unsigned int cnt);
static void history_move_up();
static void history_move_down();

static void vt100_clear_from_cursor();
static void vt100_cursor_lmove(int move_count);
static void vt100_cursor_rmove(int move_count);

//*****************************************************************************/
//
// Shell "Worker" function prototypes (ln. 174)
//
//*****************************************************************************/

static void process_in_char(char c);
void handleCommand(char * cmd);

//*****************************************************************************/
//
// Shell context data structure declaration
//
//*****************************************************************************/

typedef enum {GET_ESC, GET_BRKT, GET_ATTR, GET_CODE} esc_states_t;

typedef struct {
	const char* cmd;
	int idx;
} history_ctx_t;


typedef struct {
    char cmdBuf[CMD_BUF_LEN];

    size_t lhsCnt;
    size_t rhsCnt;

    esc_states_t esc_seq;

    history_ctx_t hist;
} shell_context_t;


static shell_context_t ctx;

void shell_buf_init()
{
    ctx.cmdBuf[CMD_BUF_LEN-1] = '\0';

    ctx.lhsCnt = 0;
    ctx.rhsCnt = 0;
}

void history_ctx_init()
{
	ctx.hist.cmd = NULL;
	ctx.hist.idx = -1;
}

void shell_ctx_init()
{
    // State variable to track escape codes
    ctx.esc_seq = GET_ESC;

	shell_buf_init();

    history_ctx_init();
}

/********************** Shell Context Helper Functions ************************/

static inline char* cmd_rhs()
{
    return (ctx.cmdBuf+CMD_MAX_LEN-ctx.rhsCnt);
}

static inline bool cmd_space_avail()
{
    return (ctx.lhsCnt+ctx.rhsCnt) <= CMD_MAX_LEN;
}

void cmd_null_terminate()
{
	// the very last character in the cmd_buffer has been set to 0
	// so rhsCnt+1 will then include the null-terminator
	memcpy(ctx.cmdBuf+ctx.lhsCnt, cmd_rhs(), ctx.rhsCnt);

    // move lhsCnt to be +cmdRhs chars forward & "reset" cmdRhs
    ctx.lhsCnt += ctx.rhsCnt;
    ctx.rhsCnt = 0;

	// Make sure the buffers comes out as a proper string from here
    ctx.cmdBuf[ctx.lhsCnt] = '\0';
}

static inline bool is_scrolling_hist()
{
    return (ctx.hist.cmd != 0);
}

static void transfer_hist_cmd()
{
    memcpy(ctx.cmdBuf, ctx.hist.cmd, ctx.lhsCnt);
    memcpy(cmd_rhs(), ctx.hist.cmd+ctx.lhsCnt, ctx.rhsCnt);

	ctx.hist.cmd = NULL;
	ctx.hist.idx = -1;
}

//*****************************************************************************/
//
// "SHELL TASK"
//
//*****************************************************************************/

// command process function just prints out the command + adds it onto the history buffer
void handleCommand(char* cmd)
{
    stdio_printf("cmd (%d) = %s", ctx.lhsCnt, cmd);

    if (strlen(cmd) > 0) {
        add_shell_cmd(cmd);
    }
}

void shell_task()
{
    // Prompt for text to be entered.
    stdio_print("\n\n \n\n=~=~=~=~=~ UART SHELL ~=~=~=~=~=\n");

    shell_hist_init();
    shell_ctx_init();

    stdio_printf(cmd_prompt);

    // Loop forever echoing data through the UART.
    while(1) {
        process_in_char(stdio_getc());
    }
}

//*****************************************************************************/
//
// SHELL WORKER FUNCTIONS!
//
//*****************************************************************************/

static void handle_cursor_move(char attr, char code)
{
    switch (code) {
    case 'A':
        history_move_up();
        break;

    case 'B':
        history_move_down();
        break;

    case 'C':
		cursor_rmove(1);
        break;

    case 'D':
        cursor_lmove(1);
        break;
    }
}

static void process_esc_sequence(char c)
{
	static char esc_attr = 0;
	
	if (ctx.esc_seq == GET_BRKT && c == '[') {
		ctx.esc_seq = GET_ATTR;
	}
	else if (ctx.esc_seq == GET_ATTR) {
		if ('0' <= c && c <= '9') {
			esc_attr = c;
			ctx.esc_seq = GET_CODE;
		}
		else {
			ctx.esc_seq = GET_ESC;
			handle_cursor_move(0, c);
		}
	}
	else if (ctx.esc_seq == GET_CODE) {
		if (c == ';') {
			// ignore any attributes prior to the semi-colon
			// and go to fetch escape code attributes
			ctx.esc_seq = GET_ATTR;
		}
		else {
			ctx.esc_seq = GET_ESC;
			handle_cursor_move(esc_attr, c);
		}
	}
	else {
		ctx.esc_seq = GET_ESC;
	}
}


// The main function - process a character whilst maintaining a state-machine to track escape character codes
static void process_in_char(char c)
{	
	if (ctx.esc_seq > GET_ESC) {
		process_esc_sequence(c);
	}
	else if (c == 0x1b) {
		ctx.esc_seq = GET_BRKT;
	}
	else {
		if (ctx.hist.idx >= 0) {
			transfer_hist_cmd();
		}

		switch (c) {
        case '\b':
			cursor_backspace();
			break;
        case 0x7F:
            cursor_delete();
            break;

        case '\r':
			if (ctx.lhsCnt || ctx.rhsCnt) {
				// Handling newline
				stdio_print("\r\n");

				// if (is_scrolling_hist()) {
				// 	memcpy(ctx.cmdBuf, ctx.hist.cmd, ctx.lhsCnt+ctx.rhsCnt+1);
				// }
				// else {
				// 	cmd_null_terminate();
				// }
				cmd_null_terminate();

				handleCommand(ctx.cmdBuf);

				shell_ctx_init();
				stdio_print(cmd_prompt);
			}
            break;

        default:
            // if (is_scrolling_hist()) {
            //     transfer_hist_cmd();
            // }

            cursor_insert_char(c);
            break;
        }
	}
}

// cursor control

static void clear_screen_and_print_cmd(const char* cmd)
{
	// can be improve by instead bringing the cursor to the left and clearing screen
    vt100_cursor_lmove(ctx.lhsCnt);
    vt100_clear_from_cursor();

    ctx.lhsCnt = stdio_print(cmd);
    ctx.rhsCnt = 0;
}


static void cursor_insert_char(char c) 
{
	// If there's space in the buffer for a new character on the cursor ctx.lhsCnt then collect char
	if (cmd_space_avail() && c >= ' ') {
		ctx.cmdBuf[ctx.lhsCnt++] = c;
		stdio_putc(c);

		// If there are chars on the cursor cmdRhs then re-send to UART
		if (ctx.rhsCnt > 0) {
			stdio_put(cmd_rhs(), ctx.rhsCnt);

			// Reset the cursor screen to match ctx.lhsCnt pos
			vt100_cursor_lmove(ctx.rhsCnt);
		}
	}
}

static void cursor_backspace()
{
	// If there are chars on the ctx.lhsCnt of the cursor, then you can delete!
	if (ctx.lhsCnt > 0) {
		vt100_cursor_lmove(1);
		ctx.lhsCnt--;

		// If there are chars on the cursor cmdRhs then re-send them to UART
		if (ctx.rhsCnt > 0) {
			stdio_put(cmd_rhs(), ctx.rhsCnt);

			// add a blank space since things got shifted to the left
			stdio_putc(' ');

			// Reset the cursor screen to match ctx.lhsCnt pos
			vt100_cursor_lmove(ctx.rhsCnt+1);
		}
		else {
			stdio_print(" \b");
		}
	}
}

static void cursor_delete()
{
	if (ctx.rhsCnt > 0) {
		ctx.rhsCnt--;

		stdio_put(cmd_rhs(), ctx.rhsCnt);

		// add a blank space since things got shifted to the left
		stdio_putc(' ');

		// Reset the cursor screen to match ctx.lhsCnt pos
		vt100_cursor_lmove(ctx.rhsCnt+1);
	}
}

static void cursor_lmove(unsigned int cnt)
{
	if (ctx.lhsCnt < cnt) {
		cnt = ctx.lhsCnt;
	}

	vt100_cursor_lmove(cnt);

	/*
	 * count changes done before because:
	 * ctx.cmdBuf[ctx.lhsCnt] atm point to the next char to add on lhs
	 * cmd_rhs() points to first char on rhs
	 * so to transfer lhs chars to the rhs the pointers need to changed before the transfer
	 */
	ctx.rhsCnt += cnt;
	ctx.lhsCnt -= cnt;

	if (!is_scrolling_hist()) {
		memcpy(cmd_rhs(), (ctx.cmdBuf+ctx.lhsCnt), cnt);
	}
}

static void cursor_rmove(unsigned int cnt)
{
	if (ctx.rhsCnt < cnt) {
		cnt = ctx.rhsCnt;
	}

	if (ctx.rhsCnt) {
		vt100_cursor_rmove(cnt);

		if (!is_scrolling_hist()) {
			memcpy((ctx.cmdBuf+ctx.lhsCnt), cmd_rhs(), cnt);
		}

		/*
		 * count changes done after because:
		 * ctx.cmdBuf[ctx.lhsCnt] atm point to the next char to add on lhs
		 * cmd_rhs() points to first char on rhs
		 * so to transfer rhs chars to the lhs the pointers need to changed after the transfer
		 */
		ctx.rhsCnt -= cnt;
		ctx.lhsCnt += cnt;
	}
}

static void history_move_up()
{
	// Scroll up Command history and keep track of current scrolling position
	if (!((ctx.hist.idx + 1) == get_hist_saved_count())) {
		// first time scrolling
		if (ctx.hist.idx < 0) {
			vt100_cursor_rmove(ctx.rhsCnt);
			cmd_null_terminate();
		}

		ctx.hist.cmd = get_shell_hist_cmd(++ctx.hist.idx);
		clear_screen_and_print_cmd(ctx.hist.cmd);
	}
}

static void history_move_down()
{
	// Scroll down Command history and keep track of current scrolling position
	// IF WE HAVE SCROLLED UP AT LEAST ONCE
	if (ctx.hist.idx >= 0) {
		ctx.hist.idx--;

		// make sure the next command down is still within the bounds of the hist buffer
		if (ctx.hist.idx >= 0) {
			ctx.hist.cmd = get_shell_hist_cmd(ctx.hist.idx);
			clear_screen_and_print_cmd(ctx.hist.cmd);
		}
		else {
			ctx.hist.cmd = NULL;
			clear_screen_and_print_cmd(ctx.cmdBuf);
		}
	}
}

//*****************************************************************************
//
// Shell "Text Control" Helper functions
//
//*****************************************************************************

// replace with clear screen
static void vt100_clear_from_cursor()
{
    stdio_print("\x1b[K");
}

static void vt100_cursor_lmove(int move_count)
{
	if (move_count <= 4) {
        // fast path for 1-4 steps back
        stdio_put("\b\b\b\b", move_count);
    } else {
        stdio_printf("\x1b[%uD", move_count);
    }
}

static void vt100_cursor_rmove(int move_count)
{
	if (move_count) {
    	stdio_printf("\x1b[%dC", move_count);
	}
}

