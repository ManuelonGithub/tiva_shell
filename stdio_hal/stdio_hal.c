

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

int stdio_init()
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

void stdio_put(const char* buf, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		stdio_putc(buf[i]);
	}
}

size_t stdio_print(const char* s)
{
	int i = 0;

	while (s[i] != '\0') {
		if (s[i] == '\n') {
			stdio_putc('\r');
		}

		stdio_putc(s[i]);
		i++;
	}

	return i;
}

void stdio_printf(const char *pcString, ...)
{
	va_list vaArgP;

	va_start(vaArgP, pcString); // Start the varargs processing.
	UARTvprintf(pcString, vaArgP);
	va_end(vaArgP); // We're finished with the varargs now.
}

