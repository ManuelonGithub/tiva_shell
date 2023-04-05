

#ifndef STDIO_HAL_EXTENSION_H_
#define STDIO_HAL_EXTENSION_H_

#include "stdio_hal.h"

void stdio_printHex(const char* s, size_t len);

void stdio_hexdump(const char* data, size_t len);

unsigned int hex_to_arr(const char* hexstr, char* dst, size_t len);

unsigned int hex_to_val(const char* hexstr);

#endif 