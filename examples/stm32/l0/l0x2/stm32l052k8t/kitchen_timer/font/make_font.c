#include <stdio.h>

#include "large_0.c"
#include "large_1.c"
#include "large_2.c"
#include "large_3.c"
#include "large_4.c"
#include "large_5.c"
#include "large_6.c"
#include "large_7.c"
#include "large_8.c"
#include "large_9.c"

#include "small_0.c"
#include "small_1.c"
#include "small_2.c"
#include "small_3.c"
#include "small_4.c"
#include "small_5.c"
#include "small_6.c"
#include "small_7.c"
#include "small_8.c"
#include "small_9.c"

#include "alarm.c"

#include "battery.c"

#include "underline.c"

void print_data(const unsigned char *p, int width, int height)
{
	int page;
	int column;
	int bit;
	unsigned char d;

	for (page = 0; page < height / 8; page++) {
		for (column = 0; column < width; column++) {
			if (column % 8 == 0)
				printf("\t");
			d = 0;
			for (bit = 0; bit < 8; bit++)
				d |= (*(p + page * 8 * width * 3 +
					bit * width * 3 + column * 3)) ? 0 :
					(1 << bit);
			printf("0x%02x,%c", d,
			       (column % 8 == 7) ? '\n' : ' ');
		}
	}
}

void print_const(unsigned char d, int width, int height)
{
	int page;
	int column;

	for (page = 0; page < height / 8; page++) {
		for (column = 0; column < width; column++) {
			if (column % 8 == 0)
				printf("\t");
			printf("0x%02x,%c", d,
			       (column % 8 == 7) ? '\n' : ' ');
		}
	}
}

int main()
{
	printf("static const char large_font[] = {\n");
	printf("\t/* 0 */\n");
	print_data(large_0.pixel_data, 32, 32);
	printf("\t/* 1 */\n");
	print_data(large_1.pixel_data, 32, 32);
	printf("\t/* 2 */\n");
	print_data(large_2.pixel_data, 32, 32);
	printf("\t/* 3 */\n");
	print_data(large_3.pixel_data, 32, 32);
	printf("\t/* 4 */\n");
	print_data(large_4.pixel_data, 32, 32);
	printf("\t/* 5 */\n");
	print_data(large_5.pixel_data, 32, 32);
	printf("\t/* 6 */\n");
	print_data(large_6.pixel_data, 32, 32);
	printf("\t/* 7 */\n");
	print_data(large_7.pixel_data, 32, 32);
	printf("\t/* 8 */\n");
	print_data(large_8.pixel_data, 32, 32);
	printf("\t/* 9 */\n");
	print_data(large_9.pixel_data, 32, 32);
	printf("};\n");

	printf("\nstatic const char small_font[] = {\n");
	printf("\t/* 0 */\n");
	print_data(small_0.pixel_data, 16, 16);
	printf("\t/* 1 */\n");
	print_data(small_1.pixel_data, 16, 16);
	printf("\t/* 2 */\n");
	print_data(small_2.pixel_data, 16, 16);
	printf("\t/* 3 */\n");
	print_data(small_3.pixel_data, 16, 16);
	printf("\t/* 4 */\n");
	print_data(small_4.pixel_data, 16, 16);
	printf("\t/* 5 */\n");
	print_data(small_5.pixel_data, 16, 16);
	printf("\t/* 6 */\n");
	print_data(small_6.pixel_data, 16, 16);
	printf("\t/* 7 */\n");
	print_data(small_7.pixel_data, 16, 16);
	printf("\t/* 8 */\n");
	print_data(small_8.pixel_data, 16, 16);
	printf("\t/* 9 */\n");
	print_data(small_9.pixel_data, 16, 16);
	printf("};\n");

	printf("\nstatic const char alarm_mark[] = {\n");
	print_data(alarm.pixel_data, 64, 16);
	printf("};\n");

	printf("\nstatic const char battery_mark[] = {\n");
	print_data(battery.pixel_data, 64, 16);
	printf("};\n");

	printf("\nstatic const char underline[] = {\n");
	print_data(underline.pixel_data, 32, 8);
	printf("};\n");

	printf("\nstatic const char allzero[] = {\n");
	print_const(0, 64, 16);
	printf("};\n");

	return 0;
}
