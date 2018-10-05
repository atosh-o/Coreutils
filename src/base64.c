/*TODO
	[ ] - When input is stdout, input should be buffered
	[ ] - We are not guaranteed that char will be 8 bits, either check
	explicitly for it, or use uint8_t

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include <getopt.h>

#define DEFAULT_COLS 76


struct option long_options[] = {
	{"decode", no_argument, NULL, 'd'},
	{"ignore-garbage", no_argument, NULL, 'i'},
	{"wrap", required_argument, NULL, 'w'},
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'v'},
	{NULL, 0, NULL, 0}
};

/* flags are global variables */
static int ignore_garbage;
static int wrap;

void usage(int status)
{
	if (status != EXIT_SUCCESS) {
		fputs("Error: Usage: bla bla\n", stderr);
	} else {
		fputs("Usage: success bla bla\n", stdout);
	}
	exit(status);
	
}

/* 0xff denotes garbage character */
unsigned char decode_char(int ch)
{
	if (ch >= 'A' && ch <= 'Z') {
		return (unsigned char) (ch -= 65);

	} else if (ch >= 'a' && ch <= 'z') {
		return (unsigned char) (ch -= 71);

	} else if (ch >= '0' && ch <= '9') {
		return ch += 4;

	} else if (ch == '+') {
		return ch = 62;

	} else if (ch == '/') {
		return ch = 63;

	} else if (isspace(ch) || ignore_garbage) {
		return ch = 0xff;

	} else {
		fprintf(stderr, "Garbage character %x\n", ch);
		usage(EXIT_FAILURE);
	}
}

void base64_decode(int argc, char *argv[])
{
	FILE *f;
	int i;
	int curr;
	int prev;

	if (argc == 0 || strcmp(*argv, "-")) /* Lazy evaluation */
		f = stdin;
	
	if ((f = fopen(*argv, "r")) == NULL) {
		fputs("Couldn't open file\n", stderr);
		usage(EXIT_FAILURE);
	}

	i = 1;
	while ((curr = fgetc(f)) != EOF) {

		curr = decode_char(curr);
		if (curr == 0xff) /* Skip garbage and '=' character */
			continue;

		switch (i) {
		case 1:
			break;
		case 2:
			fputc((prev << 2) | (curr >> 4), stdout);
			break;
		case 3:
			fputc((prev << 4) | (curr >> 2), stdout);
			break;
		case 4:
			fputc((prev << 6) | curr, stdout);
		}

		prev = curr;
		if (++i > 4)
			i = 1;
	}
}

void char_encode(char n)
{
	static int indx = 0;

	if (wrap && indx == wrap) {
		indx = 0;
		fputc('\n', stdout);
	}

	n &= 0x3f; /*clear top 2 bits */
	if (n <= 25) {
		putchar(n + 65);
	} else if ( n > 25 && n <= 51 ) {
		putchar(n + 71);
	} else if (n > 51 && n <= 61) {
		putchar(n - 4);
	} else if (n == 62) {
		putchar('+');
	} else if (n == 63) {
		putchar('/');
	}
	indx++;
}

void base64_encode(int argc, char *argv[])
{
	FILE *f;
	int i;
	int curr;
	char prev;

	if (argc == 0 || strcmp(*argv,"-") == 0) {
		f = stdin;
	} else {
		if ((f = fopen(*argv, "r")) == NULL) {
			fputs("Error: Couldn't open file\n", stdout);
			usage(EXIT_FAILURE);
		}
	}

	i = 1;
	while ((curr = getc(f)) != EOF) {

		switch (i) {
		case 1:
			char_encode(curr >> 2);
			break;
		case 2:
			char_encode((curr >> 4) | (prev << 4));
			break;
		case 3:
			char_encode((curr >> 6) | (prev << 2));
			char_encode(curr);
		}

		prev = curr;
		/* swich (i++ % 3) might overflow i, so this is better */
		if (++i > 3)
			i = 1;
		
	}

	/* paddings */
	if (i == 2) {
		char_encode(prev << 4);
		fputs("==", stdout);
	}

	if (i == 3) {
		char_encode(prev << 2);
		fputc('=', stdout);
	}

	/* newline */
	puts("");

}

int main(int argc, char *argv[])
{

	int ch;
	int decode;
	char *tmp;

	decode = 0;
	wrap = DEFAULT_COLS;

	while ((ch = getopt_long(argc, argv, "diw:", 
				long_options, NULL)) != -1) {

		switch (ch) {
		case 'd':
			decode = 1;
			break;
		case 'i':
			ignore_garbage = 1;
			break;
		case 'w':
			wrap = strtol(optarg, &tmp, 10);
			/* error checking strtol */
			if (optarg == tmp || *tmp != '\0') {
				fputs("Usage: invalid wrap size\n",
							stderr);
				exit(EXIT_FAILURE);
			}
			break;
		case 'h':
			usage(EXIT_SUCCESS);
		case 'v':
			puts("Version: 0.1");
			exit(EXIT_SUCCESS);
		}
	}

	argv += optind;
	argc -= optind;

	if (argc > 1)
		usage(EXIT_FAILURE);

	if (decode) {
		base64_decode(argc, argv);
	} else {
		base64_encode(argc, argv);
	}

	exit(EXIT_SUCCESS);
}
