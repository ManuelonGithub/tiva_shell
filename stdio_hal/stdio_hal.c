

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
#ifdef STDIO_BIG_HEX
	const char hex_lut[] = "0123456789ABCDEF";
#else
	const char hex_lut[] = "0123456789abcdef";
#endif // STDIO_BIG_HEX

	for (size_t i = 0; i < len; i++) {
		stdio_putc(hex_lut[(s[i] >> 4)]);
		stdio_putc(hex_lut[(s[i] & 0x0f)]);
		stdio_putc(' ');
	}
}