

#ifndef SHELL_READLINE_H_
#define SHELL_READLINE_H_

extern const char* shell_line_prompt;

extern size_t find_or_print_matches (const char* cmd_text, size_t len, const char** out_cmd);

char* shell_readline();

#endif // !SHELL_READLINE_H_

