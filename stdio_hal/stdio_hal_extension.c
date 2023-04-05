

#include <stdbool.h>

#include "stdio_hal_extension.h"

#ifdef STDIO_BIG_HEX
	static const char hex_lut[] = "0123456789ABCDEF";
#else
	static const char hex_lut[] = "0123456789abcdef";
#endif // STDIO_BIG_HEX

void stdio_printHex(const char* s, unsigned int len)
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

void stdio_hexdump(const char* data, unsigned int len)
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

static inline char my_toupper(char c) 
{
	return (c >= 'a') ? c - 0x20 : c; 
}

unsigned int hex_to_arr(const char* hexstr, char* dst, size_t len)
{
	char msnib = hexstr[0];
	char lsnib = hexstr[1];

	int str_idx = 1;
	int i = 0;

	while (msnib != 0 && lsnib != 0 && i < len) {
		if (msnib == ' ') {
			msnib = lsnib;
			lsnib = hexstr[++str_idx];
		}
		else {
			if ('0' <= msnib && msnib <= '9') {
				dst[i] = (msnib - '0') << 4;
			}
			else {
				msnib = my_toupper(msnib);
				if ('A' <= msnib && msnib <= 'F') {
					dst[i] = (msnib - ('A' - 10)) << 4;
				}
				else {
					break;
				}
			}

			if ('0' <= lsnib && lsnib <= '9') {
				dst[i] |= (lsnib - '0');
			}
			else {
				lsnib = my_toupper(lsnib);
				if ('A' <= lsnib && lsnib <= 'F') {
					dst[i] |= (lsnib - ('A' - 10));
				}
				else {
					break;
				}
			}

			i++;
			msnib = hexstr[++str_idx];
			lsnib = hexstr[++str_idx];
		}
	}

	return i;
}

unsigned int hex_to_val(const char* hexstr)
{
	unsigned int val = 0;

	int nibs = 0, i = 0;

	char c = hexstr[i];

	while (c != 0 && nibs < 8) {
		if (c != ' ') {
			if ('0' <= c && c <= '9') {
				c = (c - '0');
			}
			else {
				c = my_toupper(c);
				if ('A' <= c && c <= 'F') {
					c = (c - ('A' - 10));
				}
				else {
					break;
				}
			}

			val = (val << 4) | c;
			
			nibs++;
		}
		c = hexstr[++i];
	}

	return val;
}
