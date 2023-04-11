# tiva_shell

Example Shell with cursor movement support (including home, end, and delete). 
It also implements tab-completion and has a shell command history implementation, although it's not persistent.

## Registering Shell Commands
This shell implementaion utilizes a mechanism to register and gather shell commands during link-time.
The way to insert shell commands is via REGISTER_CMD macro and has to be done in the global scope (not inside a function body).

This method defines a region in the DATA section where all shell command registrations will be pooled into.
See tm4c123g/tm4c123g.ld for more details

## Porting
This shell command implementation should be easily ported and has an open license to do so.
Things to look out for:
	* Currently the shell_task() readline() functions are fully blocking, and need to be slightly modified to be RTOS/interrupt-based
	* stdio_hal/ is used to abstract away specific calls to the terminal output. These functions should all be easily ported to any target you have.
		* Note that stdio_print returns the length of the null-terminated string passed into it, and it's return value is used throughout the input processing.
	* The shell command registration implemented will most likely need to be ported to be compatible to your target platform.
		* It's a bad implementation in that regard, but I wanted to explore a link-time solution


The terminal hinges on the interfacing terminal to use VT100 codes. If you don't know, most likely you don't have to worry about it :)
