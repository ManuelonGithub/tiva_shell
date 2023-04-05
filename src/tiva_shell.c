
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
	//	When set the PLL division to be 2.5
	//	And then by default this value gets divided by 2
	// 	(400M / 2.5) / 2 = 80M
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ);
#else
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_XTAL_16MHZ);
#endif

	stdio_init();
}

int main(void)
{
	init();

	shell_task();

	while(1)
	{
	}
}

void say_hello(int argc, char** argv)
{
	stdio_print("HELLO :D!\n");
}

void say_goodbye(int argc, char** argv)
{
	stdio_print("Goodbye :(...\n");
}

REGISTER_CMD("hello", say_hello, "says hello");
REGISTER_CMD("goodbye", say_goodbye, "says goodbye");


// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif