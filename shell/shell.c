


#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "stdio_hal/stdio_hal.h"
#include "shell.h"
#include "shell_cfg.h"
#include "shell_history.h"
#include "readline.h"

//*****************************************************************************/
//
// "SHELL TASK"
//
//*****************************************************************************/

void shell_task()
{
    // Prompt for text to be entered.
    stdio_print("\n\n \n\n=~=~=~=~=~ UART SHELL ~=~=~=~=~=\n");

    shell_hist_init();

    // Loop forever echoing data through the UART.
    while(1) {
        char* cmd = readline();

		stdio_printf("cmd = %s", cmd);
		add_shell_cmd(cmd);
    }
}
