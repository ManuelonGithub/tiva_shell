#ifndef STDIO_HAL_H_
#define STDIO_HAL_H_

#include <string.h>

enum STDIO_ERR_CODES {STDIO_OK, STDIO_ERR};

int		stdio_init();

char 	stdio_getc();

void 	stdio_putc(char c);
void 	stdio_put(const char* s, size_t len);

size_t 	stdio_print(const char* s);
void	stdio_printf(const char *pcString, ...);

void 	stdio_printHex(const char* s, size_t len);
void 	stdio_hexdump(const char* data, size_t len);

#endif // !STDIO_HAL_H_