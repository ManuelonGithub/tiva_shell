

#include "shell.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "stdio_hal/stdio_hal.h"
#include "shell.h"
#include "shell_cfg.h"
#include "shell_history.h"

#define ESC 0x1B
#define TAB '\t'
#define  BS 0x7F

#define CMD_BUF_LEN CMD_MAX_LEN+1

const char* cmd_prompt = "\n >> ";

//*****************************************************************************
//
// Shell "Text Control" Helper function prototypes (ln. 318)
//
//*****************************************************************************

typedef enum {CURSOR_UP, CURSOR_DOWN, CURSOR_RIGHT, CURSOR_LEFT, CURSOR_ERR} cursor_movement_e;

cursor_movement_e cursor_move_from_esc_char(char c);

void shell_clear_chars(int clear_count);
void shell_move_cursor_left(int move_count);
void shell_move_cursor_right(int move_count);

//*****************************************************************************/
//
// Shell "Worker" function prototypes (ln. 174)
//
//*****************************************************************************/

void handleInputChar(char c);
void handleCommand(const char * cmd);

//*****************************************************************************/
//
// Shell context data structure declaration
//
//*****************************************************************************/

typedef enum {NORMAL, CURSED, TABBED, HIST, ESCAPE, FINALE} processStates_e;
typedef enum {GET_ESC, GET_BRKT, GET_CODE} esc_states_t;

typedef struct {
    char cmdBuf[CMD_BUF_LEN];
    size_t cnt;

} shell_context_t;


static shell_context_t ctx;

/********************** Shell Context Helper Functions ************************/

void shell_ctx_init()
{
    // State variable to track escape codes
    ctx.cmdBuf[CMD_BUF_LEN-1] = '\0';
    ctx.cnt = 0;
}

//*****************************************************************************/
//
// "SHELL TASK"
//
//*****************************************************************************/

void shell_task()
{
    // Prompt for text to be entered.
    stdio_printf("\n\n \n\n=~=~=~=~=~ UART SHELL ~=~=~=~=~=\n");

    shell_hist_init();
    shell_ctx_init();

    stdio_printf(cmd_prompt);

    // Loop forever echoing data through the UART.
    while(1) {
        handleInputChar(stdio_getc());
    }
}

void handleInputChar(char c)
{
    if (c == '\r') {
        stdio_print("\\r ");
    }
    else if (c == '\n') {
        stdio_print("\\n ");
    }
    else if (c == '\0') {
        stdio_print("\\0 ");
    }
    else if (c == '\t') {
        stdio_print("\\t ");
    }
    else if (c == '\t') {
        stdio_print("\\t ");
    }
    else if (c == 0x08) {
        stdio_print("BS ");
    }
    else if (c == 0x1b) {
        stdio_print("ESC ");
    }
    else if (c > 31 && c < 127) {
        stdio_putc(c);
    }
    else {
        stdio_printHex(&c, 1);
    }
}

