/*
 * http.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of ethernet_camera.
 *
 * ethernet_camera is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ethernet_camera is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ethernet_camera.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>

#include <gpio.h>
#include <nvic.h>
#include <vector.h>

#include "ethernet_camera.h"

/*
 * Hypertext Transfer Protocol (HTTP)
 * RFC 7230 - 7234
 *
 *             request  >
 * user agent ======================= origin server
 * (client)             <   response  (server)
 *
 * HTP-message = start-line
 *               *( header-field CRLF )
 *               CRLF
 *               [ message-body ]
 *
 * start-line = request-line / status-line
 *
 * request-line = method SP request-target SP HTTP-version CRLF
 *
 * status-line = HTTP-version SP status-code SP reason-phrase CRLF
 */

#define BUFSIZE			256
#define HTML_BUF_SIZE		1024
#define CAMERA_DATA_SIZE	1024 /* <= HTML_BUF_SIZE, 16 byte boundary */

extern volatile int rcvlen;

bool connection_close;
static int offset;
static char method[8];
static char request_target[16];
static int http_version_major;
static int http_version_minor;
static int content_length;
static bool encode_chunked;
static char buf[BUFSIZE];
static char html_buf[HTML_BUF_SIZE];

static char *http_gets(char *s, int size)
{
	int i;
	char prev;
	char c;

	i = 0;
	prev = '\0';
	nvic_disable_irq(NVIC_EXTI2_3);
	while (offset < rcvlen) {
		c = w5200_getc(offset++);
		if (i < size - 1)
			*(s + i++) = c;
		if (c == '\n' && prev == '\r')
			break;
		prev = c;
	}
	nvic_enable_irq(NVIC_EXTI2_3);
	if (i == 0)
		return NULL;
	*(s + i) = '\0';
	return s;
}

static void http_free(int size)
{
	nvic_disable_irq(NVIC_EXTI2_3);
	rcvlen -= size;
	w5200_free_rxmem(size);
	nvic_enable_irq(NVIC_EXTI2_3);
}

static int http_send(char *addr, int size)
{
	extern volatile int sndlen;
	extern volatile enum tcp_state tcp_state;

	nvic_disable_irq(NVIC_EXTI2_3);
	sndlen = size;
	w5200_write_txmem(addr, size);
	nvic_enable_irq(NVIC_EXTI2_3);
	while (sndlen && tcp_state == TCP_ESTABLISHED)
		;
	if (tcp_state != TCP_ESTABLISHED)
		return -1;
	gpio_toggle(GPIO_PA0);
	return size;
}

static int headlen(const char *response)
{
	int i;

	for (i = 0; *(response + i) != '\0'; i++) {
		if (strncmp(response + i, "\r\n\r\n", 4) == 0)
			return i + 4;
	}
	return 0;
}

static void http_response_index_html(bool head)
{
	int n;
	char *p;
	char tmp[20];
	extern char netinfo[];
#ifdef DEBUG
	int i;
#endif
	static const char index_html[] =
		"<!DOCTYPE html>"
		"<html>"
		"<head>"
		"<title>ethernet_camera</title>"
		"</head>"
		"<body>"
		"<h1>ethernet_camera</h1>"
		"<table width=\"50%\">"
		"<tr><td>IP address</td><td>  0.  0.  0.  0</td></tr>"
		"<tr><td>Subnet mask</td><td>  0.  0.  0.  0</td></tr>"
		"<tr><td>Gateway</td><td>  0.  0.  0.  0</td></tr>"
		"<tr><td>MAC address</td><td>00:00:00:00:00:00</td></tr>"
		"</table>"
		"<hr align=\"left\">"
		"<p><img src=\"camera.jpg\" width=\"640\" height=\"480\" "
		"alt=\"Photo image\"></p>"
		"</body>"
		"</html>";

	n = sprintf(html_buf, "HTTP/1.1 200 OK\r\n");
	n += sprintf(&html_buf[n], "Content-Type: text/html\r\n");
	n += sprintf(&html_buf[n], "Content-Length: %d\r\n\r\n",
		     sizeof(index_html) - 1);
	if (!head) {
		p = &html_buf[n];
		bcopy(index_html, p, sizeof(index_html) - 1);
		n += sizeof(index_html) - 1;
		p = strstr(p, "</td><td>");
		if (!p)
			return;
		sprintf(tmp, "%3u.%3u.%3u.%3u",
			(unsigned char)netinfo[14],
			(unsigned char)netinfo[15],
			(unsigned char)netinfo[16],
			(unsigned char)netinfo[17]);
		bcopy(tmp, p + 9, 15);
		p = strstr(p + 9, "</td><td>");
		if (!p)
			return;
		sprintf(tmp, "%3u.%3u.%3u.%3u",
			(unsigned char)netinfo[4],
			(unsigned char)netinfo[5],
			(unsigned char)netinfo[6],
			(unsigned char)netinfo[7]);
		bcopy(tmp, p + 9, 15);
		p = strstr(p + 9, "</td><td>");
		if (!p)
			return;
		sprintf(tmp, "%3u.%3u.%3u.%3u",
			(unsigned char)netinfo[0],
			(unsigned char)netinfo[1],
			(unsigned char)netinfo[2],
			(unsigned char)netinfo[3]);
		bcopy(tmp, p + 9, 15);
		p = strstr(p + 9, "</td><td>");
		if (!p)
			return;
		sprintf(tmp, "%02x:%02x:%02x:%02x:%02x:%02x",
			(unsigned char)netinfo[8],
			(unsigned char)netinfo[9],
			(unsigned char)netinfo[10],
			(unsigned char)netinfo[11],
			(unsigned char)netinfo[12],
			(unsigned char)netinfo[13]);
		bcopy(tmp, p + 9, 17);
	}
#ifdef DEBUG
	printf("send %d bytes\n", n);
	for (i = 0; i < n; i++)
		printf("%c", html_buf[i]);
	printf("\n");
#endif
	http_send(html_buf, n);
 }

static void http_response_favicon_ico(bool head)
{
	int n;
#ifdef DEBUG
	int i;
#endif
	extern const char favicon_ico_response[];
	extern const int favicon_ico_response_size;

	n = head ? headlen(favicon_ico_response) : favicon_ico_response_size;
#ifdef DEBUG
	printf("send %d bytes\n", n);
	for (i = 0; i < headlen(favicon_ico_response); i++)
		printf("%c", favicon_ico_response[i]);
#endif
	http_send((char *)favicon_ico_response, n);
}

static void http_response_camera_jpg(bool head)
{
	int s;
	int len;
	int n;
#ifdef DEBUG
	int i;
#endif
	int a;
	
	s = camera_fbuf_ctrl_stop();
	if (s) {
		gpio_clear(GPIO_PA1);
		return;
	}
	len = camera_get_fbuf_len();
	if (len < 0) {
		gpio_clear(GPIO_PA1);
		return;
	}
	n = sprintf(html_buf, "HTTP/1.1 201 Created\r\n");
	n += sprintf(&html_buf[n], "Content-Type: image/jpeg\r\n");
	n += sprintf(&html_buf[n], "Content-Length: %d\r\n\r\n", len);
#ifdef DEBUG
	printf("send %d bytes\n", n);
	for (i = 0; i < n; i++)
		printf("%c", html_buf[i]);
#endif
	if (http_send(html_buf, n) < 0)
		return;
	if (!head) {
		for (a = 0; a < len; a += n) {
			n = (len - a > CAMERA_DATA_SIZE) ? CAMERA_DATA_SIZE :
				(len - a);
			s = camera_read_fbuf(a, n, html_buf);
			if (s < 0) {
				gpio_clear(GPIO_PA1);
				return;
			}
			if (http_send(html_buf, n) < 0)
				break;
#ifdef DEBUG			
			printf("send %d bytes\n", n);
#endif
		}
	}
	s = camera_fbuf_ctrl_resume();
	if (s)
		gpio_clear(GPIO_PA1);
}

static void http_error_not_found(bool head)
{
	int n;
#ifdef DEBUG
	int i;
#endif
	static const char not_found_error[] =
		"HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 15\r\n"
		"\r\n"
		"Page not found.";

	n = head ? headlen(not_found_error) :
		((int)sizeof(not_found_error) - 1);
#ifdef DEBUG
	printf("send %d bytes\n", n);
	for (i = 0; i < n; i++)
		printf("%c", not_found_error[i]);
#endif
	http_send((char *)not_found_error, n);
}

static void http_error_version(bool head)
{
	int n;
#ifdef DEBUG
	int i;
#endif
	static const char version_error[] =
		"HTTP/1.1 505 HTTP Version Not Supported\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 28\r\n"
		"\r\n"
		"HTTP version not supported.\n";

	n = head ? headlen(version_error) : ((int)sizeof(version_error) - 1);
#ifdef DEBUG
	printf("send %d bytes\n", n);
	for (i = 0; i < n; i++)
		printf("%c", version_error[i]);
#endif
	http_send((char *)version_error, n);
}

static void http_error_bad_request(bool head)
{
	int n;
#ifdef DEBUG
	int i;
#endif
	static const char bad_request[] =
		"HTTP/1.1 400 Bad Request\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 16\r\n"
		"\r\n"
		"Invalid request\n";

	n = head ? headlen(bad_request) : ((int)sizeof(bad_request) - 1);
#ifdef DEBUG
	printf("send %d bytes\n", n);
	for (i = 0; i < n; i++)
		printf("%c", bad_request[i]);
#endif
	http_send((char *)bad_request, n);
}

static void http_error_method(bool head)
{
	int n;
#ifdef DEBUG
	int i;
#endif
	static const char method_error[] =
		"HTTP/1.1 405 Method Not Allowed\r\n"
		"Allow: GET, HEAD\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 20\r\n"
		"\r\n"
		"Method not allowed.\n";

	n = head ? headlen(method_error) : ((int)sizeof(method_error) - 1);
#ifdef DEBUG
	printf("send %d bytes\n", n);
	for (i = 0; i < n; i++)
		printf("%c", method_error[i]);
#endif
	http_send((char *)method_error, n);
}

static int request_line(char *p)
{
	int n;
	char *m;
	char *t;
	char *v;
	int i;
	static const char methods[8][8] = {
		"GET",
		"HEAD",
		"POST",
		"PUT",
		"DELETE",
		"CONNECT",
		"OPTIONS",
		"TRACE"
	};

#ifdef DEBUG
	puts(p);
#endif
	n = strlen(p);
	m = strtok(p, " ");
	t = strtok(NULL, " ");
	v = strtok(NULL, " ");
	if (m == NULL || t == NULL || v == NULL || strtok(NULL, " "))
		goto request_error;
	if (strncmp(v, "HTTP/", 5) ||
	    *(v + 5) < '0' || *(v + 5) > '9' ||
	    *(v + 6) != '.' ||
	    *(v + 7) < '0' || *(v + 7) > '9' ||
	    strncmp(v + 8, "\r\n", 2))
		goto request_error;
	http_version_major = *(v + 5) - '0';
	http_version_minor = *(v + 7) - '0';
	if (http_version_major != 1) {
		http_error_version(false);
		return -1;
	}
	for (i = 0; i < 8; i++) {
		if (strcmp(m, methods[i]) == 0)
			break;
	}
	if (i >= 8)
		goto request_error;
	strncpy(method, m, sizeof(method));
	strncpy(request_target, t, sizeof(request_target));
	return n;

request_error:
	http_error_bad_request(false);
	return -1;
}

static int header_field(char *p)
{
	int n;
	char *name;
	char *value;
	int i;
	char *last;
	char *e;

#ifdef DEBUG
	puts(p);
#endif
	n = strlen(p);
	if (n < 2 || *(p + n - 2) != '\r' || *(p + n - 1) != '\n')
		goto header_error;
	if (n == 2)
		return n;
	name = strtok(p, ":");
	value = strtok(NULL, "");
	if (name == NULL || value == NULL)
		goto header_error;
	if (strcasecmp(name, "Transfer-Encoding") == 0) {
		last = NULL;
		e = strtok(value, ", \t\r\n");
		while (e) {
			last = e;
			e = strtok(NULL, ", \t\r\n");
		}
		if (last == NULL)
			goto header_error;
		encode_chunked = (strcasecmp(last, "chunked") == 0) ? true :
			false;
	}
	if (strcasecmp(name, "Content-Length") == 0) {
		value = strtok(value, " \t\r\n");
		if (value == NULL || strtok(NULL, " \t\r\n"))
			goto header_error;
		i = strtol(value, &e, 10);
		if (*e != '\0' || i < 0 || i >= 0x7fffffff ||
		    (content_length && content_length != i))
			goto header_error;
		content_length = i;
	}
	if (strcasecmp(name, "Connection") == 0) {
		value = strtok(value, " \t\r\n");
		if (value == NULL || strtok(NULL, " \t\r\n"))
			goto header_error;
		if (strcasecmp(value, "close") == 0)
			connection_close = true;
		else if (strcasecmp(value, "keep-alive") == 0)
			connection_close = false;
		else
			goto header_error;
	}
	return n;

header_error:
	http_error_bad_request(false);
	return -1;
}

static int message_body(void)
{
	int size;
	int n;
	char *s;
	char *e;
	int i;

#ifdef DEBUG
	puts("message-body");
#endif
	if (!encode_chunked) {
		if (content_length > rcvlen - offset) {
#ifdef DEBUG
			printf("content_length = %d, rcvlen - offset = %d\n",
			       content_length, rcvlen - offset);
#endif
			goto message_error;
		}
		return content_length;
	}
	size = 0;
	do {
		if (http_gets(buf, BUFSIZE) == NULL)
			goto message_error;
		n = strlen(buf);
		if (n < 2 || buf[n - 2] != '\r' || buf[n - 1] != '\n')
			goto message_error;
		size += n;
		s = strtok(buf, ";\r\n");
		if (s == NULL)
			goto message_error;
		i = strtol(s, &e, 16);
		if (*e != '\0' || i < 0 || i >= 0x7fffffff)
			goto message_error;
#ifdef DEBUG
		printf("chunk-size = %d\n", i);
#endif
		if (i) {
			if (i > rcvlen - offset)
				goto message_error;
			offset += i;
			if (http_gets(buf, BUFSIZE) == NULL)
				goto message_error;
			if (strcmp(buf, "\r\n"))
				goto message_error;
			size += i + 2;
		}
	} while (i);
	do {
		if (http_gets(buf, BUFSIZE) == NULL)
			goto message_error;
		n = strlen(buf);
		if (n < 2 || buf[n - 2] != '\r' || buf[n - 1] != '\n')
			goto message_error;
#ifdef DEBUG
		printf("%s", buf);
#endif
		size += n;
	} while (strcmp(buf, "\r\n"));
	return size;

message_error:
	http_error_bad_request(false);
	return -1;
}

int http_proc(void)
{
	int n;
	int size;
	
	gpio_toggle(GPIO_PA0);
	offset = 0;
	n = request_line(http_gets(buf, BUFSIZE));
	if (n < 0)
		return n;
	size = n;
	encode_chunked = false;
	content_length = 0;
	connection_close = (http_version_minor == 0) ? true : false;
	do {
		if (http_gets(buf, BUFSIZE) == NULL) {
			http_error_bad_request(false);
			return -1;
		}
		n = header_field(buf);
		if (n < 0)
			return n;
		size += n;
	} while (n > 2);	/* !CRLF */
	if (encode_chunked || content_length) {
		n = message_body();
		if (n < 0)
			return n;
		size += n;
	}
	http_free(size);
	if (strcmp(method, "GET") && strcmp(method, "HEAD")) {
		http_error_method(false);
		return size;
	}
	if (strcmp(request_target, "/") == 0 ||
	    strcmp(request_target, "/index.html") == 0) {
		http_response_index_html(strcmp(method, "HEAD") == 0);
		return size;
	}
	if (strcmp(request_target, "/favicon.ico") == 0) {
		http_response_favicon_ico(strcmp(method, "HEAD") == 0);
		return size;
	}
	if (strcmp(request_target, "/camera.jpg") == 0) {
		http_response_camera_jpg(strcmp(method, "HEAD") == 0);
		return size;
	}
	http_error_not_found(strcmp(method, "HEAD") == 0);
	return size;
}
