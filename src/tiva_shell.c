
/*************************************************************************************************/
/**
 * @file    tiva_shell.c
 * @brief   Example of a shell with cursor movement and history
 * @author  ManuelonGithub
 * 
 * @copyright   Copyright (c) 2023 ManuelonGithub. 
 *              This project is under the MIT License.
*/
/*************************************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"

#include "stdio_hal/stdio_hal.h"
#include "stdio_hal/stdio_hal_extension.h"
#include "shell/shell.h"

// #define CLK_80M

void init()
{

#ifdef CLK_80M
	// Theory of Operation:
	//	The PLL clock will run at 400MHz when the 16MHz external crystal is connected to it
	// 	then by default this value gets divided by 2
	//	and then we set the PLL division to be 2.5
	// 	(400M / 2) / 2.5 = 80M
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ);
#else
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_XTAL_16MHZ);
#endif

	stdio_init();
}

/**************************************************************************************************/
/**************************************************************************************************/

int main(void)
{
	init();

	shell_task();

	while(1)
	{
	}
}

/**************************************************************************************************/
/**************************************************************************************************/

static void hello(int argc, char** argv)
{
	stdio_print("HELLO :D!\n");
}

static void goodbye(int argc, char** argv)
{
	stdio_print("Goodbye :(...\n");
}

REGISTER_CMD("hello", hello, "says hello");
REGISTER_CMD("goodbye", goodbye, "says goodbye");

static void sum(int argc, char** argv)
{
	int sum = 0;
	for (int i = 0; i < argc; i++) {
		int val = 0;
		char* check;
		
		val = strtol(argv[i], &check, 10);
		if (argv[i] == check) {
			goto usage;
		}

		sum +=val;
	}

	stdio_printf("sum: %d\n", sum);
	return;

usage:
	stdio_print("usage: sum ## ## ## ... all signed 32-bit decimal");
}

REGISTER_CMD("sum", sum, "sums signed 32-bit decimal numbers. Limited by max arg count");

/**************************************************************************************************/
/**************************************************************************************************/

// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif