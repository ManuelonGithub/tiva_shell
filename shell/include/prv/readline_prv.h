

// input guard purposely not inserted
// this header is meant solely for realine

// I just have so many private static function prototypes ...
// so I decided to add them here where I can properly document then
// without making the readline file gigantic

/**
 * @brief 	Entry function for input character processing
 * @param 	c input character to process
 * @return 	true - if a complete string is present after a "newline" has been processed
 */
static bool process_char(char c);

/**
 * @brief 	Performs the actions related to a escape code + attribute
 * @param 	attr escape code attribute
 * @param 	code escape code
 * @details	escape code: x1b[<attr><code>
 */
static void handle_esc_code(char attr, char code);

/// @brief	 inserts a new character to the left of the cursor
/// @param c new character to insert
static void cursor_insert_char(char c);

/// @brief removes a character to the left of the cursor
static void cursor_backspace();

/// @brief removes a character to the right of the cursor
static void cursor_delete();

/// @brief moves the cursor to the left
/// @param cnt amount of spaces to move
static void cursor_lmove(unsigned int cnt);

/// @brief 		moves the cursor to the right
/// @param cnt 	amount of spaces to move
static void cursor_rmove(unsigned int cnt);

/// @brief iterate up the saved command history
static void history_move_up();

/// @brief iterate down the saved command history
static void history_move_down();

/// @brief 		Clears the the current line and sets it to cmd
/// @param 	cmd text to the set the line to after it's been cleared
static void cursor_line_reset(const char* cmd);

/// @brief combines the text on either side of the cursor and null-terminates the command
static void cmd_null_terminate();

/// @brief transfers the history command being displayed into the command buffer
static void transfer_hist_cmd();

/// @brief VT100 command to erase all chars to the right of the cursor
static void vt100_clear_from_cursor();

/// @brief 	   VT100 command to move the cursor left 
/// @param cnt amount of the times to move the cursor
static void vt100_cursor_lmove(int cnt);

/// @brief 	   VT100 command to move the cursor left 
/// @param cnt amount of the times to move the cursor
static void vt100_cursor_rmove(int cnt);