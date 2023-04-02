

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

static void format_hexdump_line(const char* data_snip, char* hex_snip, char* ascii_snip, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        hex_snip[i*3] = hex_lut[(data_snip[i] >> 4) & 0x0f];
        hex_snip[i*3+1] = hex_lut[data_snip[i] & 0x0f];
        hex_snip[i*3+2] = ' ';
        ascii_snip[i] = (data_snip[i] > 31 && data_snip[i] < 127) ? data_snip[i] : '.';
    }
}

void stdio_hexdump(const char* data, size_t len)
{
    char hex_snip[3 * 16 + 1] = {0};
    char ascii_snip[16 + 1] = {0};

	const size_t hex_lines = len >> 4;
    const size_t leftover = len & 0x0f;

    for (size_t i = 0; i < hex_lines; i++) {
        const char* data_snip = data + (i << 4);
        format_hexdump_line(data_snip, hex_snip, ascii_snip, 16);
        stdio_printf("0x%03x0: %s | %s\n", i, hex_snip, ascii_snip);
    }

    if (leftover) {
        const char* data_snip = data + (hex_lines << 4);
        format_hexdump_line(data_snip, hex_snip, ascii_snip, leftover);

		if (hex_lines) {
        	memset(hex_snip + leftover * 3, ' ', (16 - leftover) * 3);
		}
		
        stdio_printf("0x%03x0: %s | %s\n", hex_lines, hex_snip, ascii_snip);
    }
}
