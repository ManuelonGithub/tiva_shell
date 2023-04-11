

#ifndef SHELL_READLINE_H_
#define SHELL_READLINE_H_

extern const char* shell_line_prompt;

/**
 * @brief 	Find one or more shell command matches of a given prefix
 * @param 	cmd_text shell command prefix
 * @param 	len length of shell command prefix
 * @param 	out_cmd shell command match IF only one match was found
 * @return 	SIZE_MAX if no matches were found
 * 			otherwise one or more matches were found. Use out_cmd to check if it was a single match
 * @note	If multiple matches are found then it'll print them via stdio,
 * 			and the terminal line needs to be reset to show the original text
 */
extern size_t find_or_print_matches(const char* cmd_text, size_t len, const char** out_cmd);

/**
 * @brief 	Will receive and process user input until a complete and valid line is digested.
 * @return	Pointer to the digested line. 
 *			Can be modified within its bounds (0 -> null terminator).
 * @details	Will handle cursor movement, shell history iteration, and auto-completion.	
 */
char* shell_readline();

#endif // !SHELL_READLINE_H_

