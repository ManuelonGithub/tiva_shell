

#ifndef SHELL_H_
#define SHELL_H_

typedef void (*shell_func_t)(int, char**);
typedef struct {
	const char* cmd_txt;
	shell_func_t func;
	const char* help_txt;
} shell_cmd_t;

/// @brief 	registers a new shell command
/// @param	text shell command text - must be unique
/// @param	func shell command function - must be unique 
/// @param	help shell command help text
#define REGISTER_CMD(text, func, help) \
	shell_cmd_t cmd_##func __attribute__((section(".shell_cmd."#func))) = {text, func, help}

void shell_task();

#endif // !SHELL_H_