

#ifndef SHELL_H_
#define SHELL_H_

typedef enum {SHELL_OK, SHELL_ERR} shell_code_t;
typedef void (*shell_func_t)(int, char**);

void shell_register(const char* name, shell_func_t func, const char* help_txt);

#define REGISTER_CMD(func, help) shell_register(#func, func, help)

void shell_task();

#endif // !SHELL_H_