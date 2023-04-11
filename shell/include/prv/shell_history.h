

#ifndef SHELL_HISTORY_H_
#define SHELL_HISTORY_H_

/// @brief Iniitalizes the internal shell history data structures
void hist_init();

/// @brief Get the current saved command history count
/// @return Number of items current;y saved in the shell history
unsigned int hist_count();

/// @brief Get a command history entry via an offset (youngest to oldest)
/// @param offset offset, ordered from youngest entry to oldest. Must be lower than hist_count()
/// @return const pointer to command entry in history
const char* hist_cmd_by_offset(unsigned int offset);

/**
 * @brief 	Add a new entry into the shell history
 * @param 	cmd new shell command to add to history
 * @note	this call can result in existing entries to be overwritten
 * 			and will discard entries that are duplicate of the youngest entry
*/
void hist_add_entry(const char* cmd);

#endif /* SHELL_HISTORY_H_ */
