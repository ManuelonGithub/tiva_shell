

#include "stdio_hal.h"

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

#include "driverlib/uart.h"

#ifndef STDIO_BAUD
	#define STDIO_BAUD 115200
#endif // !STDIO_UART_BAUD

#ifdef UART_BUFFERED
	#undef UART_BUFFERED
#endif	// !UART_BUFFERED

#include "utils/uartstdio.h"

#ifdef STDIO_BIG_HEX
	static const char hex_lut[] = "0123456789ABCDEF";
#else
	static const char hex_lut[] = "0123456789abcdef";
#endif // STDIO_BIG_HEX

stdio_code_t stdio_init()
{
	// Enable UART peripheral & the GPIO Peripheral where the uart pinout is connected
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	// Set GPIO A0 and A1 as UART pins.
	ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configure the UART with the inputed speed + 8-N-1 operation.
	UARTStdioConfig(0, STDIO_BAUD, MAP_SysCtlClockGet());

    return STDIO_OK;
}

char stdio_getc()
{
	return (char)UARTgetc();
}

void stdio_putc(char c)
{
	MAP_UARTCharPut(UART0_BASE, c);
}

size_t stdio_put(const char* buf, size_t len)
{
	return UARTwrite(buf, len);
}

size_t stdio_print(const char* s)
{
	int i = 0;

	while (s[i] != '\0') {
		stdio_putc(s[i]);
		i++;
	}

	return i;
}

void stdio_printf(const char *pcString, ...)
{
	va_list vaArgP;

	//
	// Start the varargs processing.
	//
	va_start(vaArgP, pcString);

	UARTvprintf(pcString, vaArgP);

	//
	// We're finished with the varargs now.
	//
	va_end(vaArgP);
}

void stdio_printHex(const char* s, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		stdio_putc(hex_lut[(s[i] >> 4)]);
		stdio_putc(hex_lut[(s[i] & 0x0f)]);
		stdio_putc(' ');
	}
}

void stdio_hexdump(const void* data, size_t len)
{
	// dontains both the hex and ascii snippet
	// requires: 	3*16 bytes + 
	//				2 bytes for the seperator + 
	//				16 bytes for the ascii snippet + 
	//				2 bytes for newline and null-termination
	// = 68 bytes
	char dump_line[68];

	dump_line[66] = '\n';
	dump_line[67] = '\0';

	dump_line[48] = '|';
	dump_line[49] = ' ';

	char* ascii_snip = dump_line+50;

	size_t hex_lines = len >> 4;
	size_t leftover = len & 0x0F;

	for (size_t i = 0; i < hex_lines; i++) {
		char* data_snip = (char*)(data+(i << 4));
		for (size_t j = 0; j < 16; j++) {
			size_t line_idx = j*3;

			if (data_snip[j] > 31 && data_snip[j] < 127) {
				ascii_snip[j] = data_snip[j];
			}
			else {
				ascii_snip[j] = '.';
			}

			dump_line[line_idx] = hex_lut[(data_snip[j] >> 4)];
			dump_line[line_idx+1] = hex_lut[(data_snip[j] & 0x0f)];
			dump_line[line_idx+2] = ' ';
		}

		stdio_printf("0x%03x0: %s", i, dump_line);
	}

	if (leftover) {
		char* data_snip = (char*)(data+(hex_lines << 4));
		size_t blanks = 16-leftover;
		ascii_snip[leftover] = '\n';
		ascii_snip[leftover+1] = '\0';

		for (size_t i = 0; i < leftover; i++) {
			size_t line_idx = i*3;

			if (data_snip[i] > 31 && data_snip[i] < 127) {
				ascii_snip[i] = data_snip[i];
			}
			else {
				ascii_snip[i] = '.';
			}

			dump_line[line_idx] = hex_lut[(data_snip[i] >> 4)];
			dump_line[line_idx+1] = hex_lut[(data_snip[i] & 0x0f)];
			dump_line[line_idx+2] = ' ';
		}

		memset(dump_line+(leftover*3), 0x20, blanks*3);
		stdio_printf("0x%03x0: %s", hex_lines+1, dump_line);
	}
}