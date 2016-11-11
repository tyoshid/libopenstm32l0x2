#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include <nvic.h>
#include <vector.h>

#define MAXTOKEN		10
#define BUFSIZE			64
#define DELIMITER		" /.,;:"
#define PROMPT			"usb_speaker> "

static void debug(void)
{
	extern volatile unsigned int recv_packet;
	extern volatile unsigned int recv_len;
	extern volatile int max_recv_len;
	extern volatile int min_recv_len;
	extern volatile unsigned int send_packet;
	extern volatile unsigned int send_len;
	extern volatile unsigned int overrun;
	extern volatile int max_sof_count;
	extern volatile unsigned int sync_error;
	extern volatile int i2c_status;	
	unsigned int tmp[10];
	
	nvic_disable_irq(NVIC_USB | NVIC_DMA_CH2_3);
	tmp[0] = recv_packet;
	tmp[1] = recv_len;
	tmp[2] = max_recv_len;
	tmp[3] = min_recv_len;
	tmp[4] = send_packet;
	tmp[5] = send_len;
	tmp[6] = overrun;
	tmp[7] = max_sof_count;
	tmp[8] = sync_error;
	tmp[9] = i2c_status;	
	nvic_enable_irq(NVIC_USB | NVIC_DMA_CH2_3);

	printf("recv_packet: %u\n", tmp[0]);
	printf("recv_len: %u\n", tmp[1]);
	printf("max_recv_len: %u\n", tmp[2]);
	printf("min_recv_len: %u\n", tmp[3]);
	printf("send_packet: %u\n", tmp[4]);
	printf("send_len: %u\n", tmp[5]);
	printf("overrun: %u\n", tmp[6]);
	printf("max_sof_count: %u\n", tmp[7]);
	printf("sync_error: %u\n", tmp[8]);
	printf("i2c_status: 0x%08x\n", tmp[9]);
}

static void help(void)
{
	puts("Usage:");
	puts("  debug   Print statistics");
	puts("  help    Print this message");
}

static void get_line(char buf[])
{
	int i;
	int c;

	i = 0;
	do {
		c = getchar();
		if (isprint(c)) {
			putchar(c);
			if (i < BUFSIZE - 1)
				buf[i++] = c;
		} else {
			switch (c) {
			case '\b':
				if (i > 0) {
					printf("\b \b");
					--i;
				}
				break;
			case '\n':
				putchar('\n');
				buf[i] = '\0';
				break;
			default:
				break;
			}
		}
	} while (c != '\n');
}

void command(void)
{
	static char buf[BUFSIZE];
	int i;
	static char *p[MAXTOKEN];

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	while (1) {
		printf(PROMPT);
		get_line(buf);
		for (i = 0; i < MAXTOKEN; i++) {
			p[i] = strtok(i ? NULL : buf, DELIMITER);
			if (p[i] == NULL)
				break;
		}
		if (i == 0)
			continue;
		if (strncasecmp(p[0], "debug", strlen(p[0])) == 0)
			debug();
		else if (strncasecmp(p[0], "help", strlen(p[0])) == 0 ||
			 strcmp(p[0], "?") == 0)
			help();
		else
			puts("unknown command");
	}
}
