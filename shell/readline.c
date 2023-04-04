


#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "stdio_hal/stdio_hal.h"
#include "shell_cfg.h"
#include "shell_history.h"

#include "readline.h"
#include "readline_prv.h"

/**************************************************************************************************/
/**************************************************************************************************/

#define CMD_BUF_LEN CMD_MAX_LEN+1

typedef enum {GET_ESC, GET_BRKT, GET_ATTR, GET_CODE} esc_states_t;

typedef struct {
    char cmdBuf[CMD_BUF_LEN];

    size_t lhsCnt;
    size_t rhsCnt;

    esc_states_t esc_seq;

    int hist_idx;
} readline_ctx_t;

static readline_ctx_t ctx;

/**************************************************************************************************/
/**************************************************************************************************/

/// @brief initializes the history index of the readline context
static inline void history_idx_init()
{
	ctx.hist_idx = -1;
}

/// @brief initializes the readline context
void readline_init()
{
    // State variable to track escape codes
    ctx.esc_seq = GET_ESC;

	ctx.cmdBuf[CMD_BUF_LEN-1] = '\0';

    ctx.lhsCnt = 0;
    ctx.rhsCnt = 0;

    history_idx_init();
}

/**************************************************************************************************/
/**************************************************************************************************/

// readline function body

const char* cmd_prompt = ">>";

char* readline()
{
	readline_init();
	stdio_printf("\n%s ", cmd_prompt);

	// Loop forever echoing data through the UART.
    while(!process_char(stdio_getc())) {}
	
	return ctx.cmdBuf;
}

/**************************************************************************************************/
/**************************************************************************************************/

/// @brief 	get pointer to the buffer space to the right of the cursor
static inline char* cmd_rhs()
{
    return (ctx.cmdBuf+CMD_MAX_LEN-ctx.rhsCnt);
}

/// @brief checks if there is space available in the buffer
static inline bool cmd_space_avail()
{
    return (ctx.lhsCnt+ctx.rhsCnt) <= CMD_MAX_LEN;
}

/// @brief checks the context if history is currently being iterated through
static inline bool browsing_history()
{
	return (ctx.hist_idx >= 0);
}

/**************************************************************************************************/
/**************************************************************************************************/

/// @brief	processes bytes from a VT100 escape sequence
/// @param 	c byte that *should* be part of an escape sequence 
static void process_esc_sequence(char c)
{
	static char esc_attr = 0;
	
	if (ctx.esc_seq == GET_BRKT && c == '[') {
		ctx.esc_seq = GET_ATTR;
	}
	else if (ctx.esc_seq == GET_ATTR) {
		// there may or may not be an attribute
		if ('0' <= c && c <= '9') {
			esc_attr = c;
			ctx.esc_seq = GET_CODE;
		}
		else {
			handle_esc_code(0, c); // skip right to handling the escape code if no attribute #
			ctx.esc_seq = GET_ESC;
		}
	}
	else if (ctx.esc_seq == GET_CODE) {
		if (c == ';') {
			ctx.esc_seq = GET_ATTR; // ignore any attributes prior to the semi-colon
		}
		else {
			handle_esc_code(esc_attr, c);
			ctx.esc_seq = GET_ESC;
		}
	}
	else {
		// Escape code "error recovery"
		ctx.esc_seq = GET_ESC;
	}
}

static bool process_char(char c)
{	
	if (ctx.esc_seq > GET_ESC) {
		process_esc_sequence(c);
	}
	else if (c == 0x1b) {
		ctx.esc_seq = GET_BRKT;
	}
	else {
		// If we get to this point we are going to modify the cmd buffer
		// so the current displayed history needs to be finally transferred to the buffer
		if (browsing_history()) {
			transfer_hist_cmd();
		}

		switch (c) {
			case '\b':	// BS
				cursor_backspace();
				break;

			case 0x7F:	// ctrl+BS
				cursor_delete();
				break;

			case '\r':	// new line
				if (ctx.lhsCnt || ctx.rhsCnt) {
					stdio_print("\r\n");
					cmd_null_terminate();
					return true;
				}
				break;

			default:	// everything else
				cursor_insert_char(c);
				break;
        }
	}

	return false;
}

static void handle_esc_code(char attr, char code)
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

/**************************************************************************************************/
/**************************************************************************************************/

static void cursor_insert_char(char c) 
{
	if (cmd_space_avail() && c >= ' ') {
		ctx.cmdBuf[ctx.lhsCnt++] = c;
		stdio_putc(c);

		// If there are chars on the cursor cmdRhs then re-send to UART
		if (ctx.rhsCnt > 0) {
			stdio_put(cmd_rhs(), ctx.rhsCnt);
			vt100_cursor_lmove(ctx.rhsCnt); // Reset the cursor screen to match ctx.lhsCnt pos
		}
	}
}

static void cursor_backspace()
{
	if (ctx.lhsCnt > 0) {
		vt100_cursor_lmove(1);
		ctx.lhsCnt--;

		// If there are chars on the cursor cmdRhs then re-send them to UART
		if (ctx.rhsCnt > 0) {
			stdio_put(cmd_rhs(), ctx.rhsCnt);
			stdio_putc(' '); // add a blank space since things got shifted to the left
			vt100_cursor_lmove(ctx.rhsCnt+1); // Reset the cursor screen to match ctx.lhsCnt pos
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
		stdio_putc(' '); // add a blank space since things got shifted to the left
		vt100_cursor_lmove(ctx.rhsCnt+1); // Reset the cursor screen to match ctx.lhsCnt pos
	}
}

static void cursor_lmove(unsigned int cnt)
{
	if (ctx.lhsCnt < cnt) {
		cnt = ctx.lhsCnt; // limit cnt #
	}

	vt100_cursor_lmove(cnt);

	// to transfer lhs chars to the rhs the pointers need to change before the transfer
	ctx.rhsCnt += cnt;	// rhsCnt is used to derive the start of the rhs
	ctx.lhsCnt -= cnt;	// lhsCnt is used to derive the end of the lhs

	if (!browsing_history()) {
		// while browsing hist the contents are not in cmdBuf yet, so no byte transfers!
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

		if (!browsing_history()) {
			// while browsing hist the contents are not in cmdBuf yet, so no byte transfers!
			memcpy((ctx.cmdBuf+ctx.lhsCnt), cmd_rhs(), cnt);
		}

		// to transfer rhs chars to the lhs the pointers need to change after the transfer
		ctx.rhsCnt -= cnt;	// rhsCnt is used to derive the start of the rhs
		ctx.lhsCnt += cnt;	// lhsCnt is used to derive the end of the lhs
	}
}

static void history_move_up()
{
	if (!((ctx.hist_idx + 1) == get_hist_saved_count())) {	// check if there's history to iterate
		// first time scrolling
		if (!browsing_history()) {
			vt100_cursor_rmove(ctx.rhsCnt);
			cmd_null_terminate();
		}
		ctx.hist_idx = ctx.hist_idx + 1; // doing it like this for some hopeful optimization
		cursor_line_reset(get_shell_hist_cmd(ctx.hist_idx));
	}
}

static void history_move_down()
{
	if (browsing_history()) {
		ctx.hist_idx--;

		// make sure the next command down is still within the bounds of the hist buffer
		if (browsing_history()) {
			cursor_line_reset(get_shell_hist_cmd(ctx.hist_idx));
		}
		else {
			cursor_line_reset(ctx.cmdBuf);
		}
	}
}

static void cursor_line_reset(const char* cmd)
{
    vt100_cursor_lmove(ctx.lhsCnt);
    vt100_clear_from_cursor();

    ctx.lhsCnt = stdio_print(cmd);
    ctx.rhsCnt = 0;
}

static void cmd_null_terminate()
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

static void transfer_hist_cmd()
{
	const char* cmd = get_shell_hist_cmd(ctx.hist_idx);

    memcpy(ctx.cmdBuf, cmd, ctx.lhsCnt);
    memcpy(cmd_rhs(), cmd+ctx.lhsCnt, ctx.rhsCnt);

	history_ctx_init();
}

/**************************************************************************************************/
/**************************************************************************************************/

static void vt100_clear_from_cursor()
{
    stdio_print("\x1b[K");
}

static void vt100_cursor_lmove(int cnt)
{
	if (cnt <= 4) {
        // fast path for 1-4 steps back
        stdio_put("\b\b\b\b", cnt);
    } else {
        stdio_printf("\x1b[%uD", cnt);
    }
}

static void vt100_cursor_rmove(int cnt)
{
	if (cnt) {
    	stdio_printf("\x1b[%dC", cnt);
	}
}

