#ifndef STDIO_HAL_H_
#define STDIO_HAL_H_

#include <string.h>

typedef enum {STDIO_OK, STDIO_ERR} stdio_code_t;

stdio_code_t	stdio_init();

char 	stdio_getc();
void 	stdio_putc(char c);
size_t 	stdio_put(const char* s, size_t len);

size_t 	stdio_print(const char* s);
void	stdio_printf(const char *pcString, ...);

void stdio_printHex(const char* s, size_t len);
void stdio_hexdump(const char* data, size_t len);

#endif // !STDIO_HAL_H_