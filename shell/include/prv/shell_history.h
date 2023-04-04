

#ifndef SHELL_HISTORY_H_
#define SHELL_HISTORY_H_

void shell_hist_init();

unsigned int get_hist_saved_count();
// unsigned int get_hist_total_count();

const char* get_shell_hist_cmd(unsigned int offset);
void add_shell_cmd(const char* cmd);


#endif /* SHELL_HISTORY_H_ */
