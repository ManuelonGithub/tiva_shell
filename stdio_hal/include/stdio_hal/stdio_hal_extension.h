

#ifndef STDIO_HAL_EXTENSION_H_
#define STDIO_HAL_EXTENSION_H_

#include "stdio_hal.h"

/// @brief Converts each byte to ASCII hex in a buffer and prints them inline via stdio
/// @param but raw byte buffer to convert
/// @param len length of byte buffer
void stdio_printHex(const char* buf, size_t len);

/// @brief Prints a formatted hexdump of a provided buffer. 
/// @param data raw byte buffer to hex-dump
/// @param len length of byte buffer
void stdio_hexdump(const char* data, size_t len);

/**
 * @brief 	Helper function converts an ASCII Hex string and into a raw byte buffer
 * @param 	hexstr ASCII Hex string to retrieve hex values from
 * @param 	dst raw byte buffer to insert hex data
 * @param 	len max length of dst
 * @return 	amount of bytes converted. Bound by either the hex string length OR 'len'
 * 
 * @note	This function will convert the ASCII Hex "inline" to the dst buffer
 * 			e.g. "deadbeef" will result in dst[0] = de, [1] = ad, [2] = be, [3] = ef
 * @note	If you view hexstr as being a single value, it'll be converted into dst in big-endian
 */
unsigned int hex_to_arr(const char* hexstr, char* dst, size_t len);

#endif 