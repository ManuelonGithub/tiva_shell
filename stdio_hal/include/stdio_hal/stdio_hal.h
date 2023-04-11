#ifndef STDIO_HAL_H_
#define STDIO_HAL_H_

#include <string.h>


/// @brief Initializes internal data structures and hardware where stdio will be directed to
void		stdio_init();

/// @brief 	get single character from stdio. Blocks until character is received.
/// @return character inputted by the user
char 	stdio_getc();

/// @brief 	put single character into stdio. Blocks if H/W is not ready to receive character
/// @param c character to output
void 	stdio_putc(char c);

/**
 * @brief	Put multiple characters into stdio. 
 *			Blocks until all characters are successfully transfered to the H/W peripheral
 * @param 	s byte buffer to transmit
 * @param 	len length of byte buffer
 */
void 	stdio_put(const char* s, size_t len);

/**
 * @brief	Print null-terminated string via stdio
 *			Blocks until all characters are successfully transfered to the H/W peripheral
 * @param 	s null-terminated string to transmit
 * @return	Amount of bytes written into stdio until a null-terminator was met
 * @note	will automatically translate /n into /r/n
 * @warning	This is a potential porting issue. 
 * 			Current shell implemation relies on the return value of this function so it 
 * 			doesn't need to both print to stdio and find length via strlen()
 */
size_t 	stdio_print(const char* s);

/**
 * @brief	Process and print formatted string via stdio
 *			Blocks until the string is processed all characters are successfully 
 * 			transfered to the H/W peripheral
 * @param 	pcString null-terminated formatted string to transmit
 * @param	... V-ARG list containing all arguments referrenced by pcString
 * @note	will automatically translate /n into /r/n
 */
void	stdio_printf(const char *pcString, ...);

#endif // !STDIO_HAL_H_