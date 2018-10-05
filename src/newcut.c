/* TODO
	[x] Implement -f -d -s
	[x] Rewrite the whole delimiter thing
	[ ] Fix the bug in -f delimiters
	[ ] Add multibyte delimiter support
	[ ] Field delimiter can't be newline

Notes after reading FreeBSD version:

	1. I could have used getc, instead of managing local buffer
	2. if *buf = 1 printf("%d - %d", *buf, *buf++) will print [soemthing]
	- 1, because right expressions are evaluated first.
	3. fgetln BSD version of getline
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <locale.h>

#include <unistd.h>

#include <getopt.h>

#define BUF_SIZE 1024

struct option long_opt[] = {
	{"bytes", required_argument, NULL, 'b'},
	{"characters", required_argument, NULL, 'c'},
	{"delimiter", required_argument, NULL, 'd'},
	{"fields", required_argument, NULL, 'f'},
	{"complement", no_argument, NULL, 'm'},
	{"only-deilimited", no_argument, NULL, 's'},
	{"output-delimiter", required_argument, NULL, 'o'},
	{"zero-terminated", no_argument, NULL, 'z'},
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'v'},
	{NULL, 0, NULL, 0}
};

struct range {
	long high;
	long low;
	struct range *next;
	struct range *prev;
};


enum operation_mode {bflag, fflag, none};

static struct range *ranges;
static char field_delimiter;
static int complement;
static int zero_terminate;
static char *output_delimiter;

/*TODO - write real error messages */
void usage(int status)
{
	if (status != EXIT_SUCCESS) {
		fputs("Usage: [program_name] [-flags blabla]\n", stderr);
	} else {
		puts("This is how you use the program, bye");
	}
	exit(status);
}

/* Implement the list parser as FSA */
#define INITIAL 0	// Init state
#define INIT_DASH 1	// [-]
#define INIT_NUM 2	// [num]
#define DASH_NUM 3	// -[num]
#define NUM_DASH 4	// num[-]
#define NUM_DASH_NUM 5	// num-[num]
#define END 6		// '\0'
/* FSA was not a good idea, function became unreadable. No wonder, neither
 * coreutils nor BSD does FSA, both versions parse text using simple if else
 * statements, ANYWAY I have to write this FSA till the end*/
void initialize_ranges(char *list)
{
	int state;
	struct range *curr;
	struct range *prev;
	int cont;

	state = INITIAL;
	cont = 1;
	curr = prev = NULL;

	while (cont) {
		switch(state){
		case INITIAL:
			prev = curr;
			curr = malloc(sizeof(struct range));
			memset(curr, 0, sizeof(struct range));
			if (!ranges)		/*first elem in linked-list*/
				ranges = curr;
			if (prev != NULL) {
				prev->next = curr;
				curr->prev = prev;
			}

			if (*list == '-') {
				list++;
				state = INIT_DASH;
			} else if (isdigit(*list)) {
				state = INIT_NUM;
			} else {
				usage(EXIT_FAILURE);
			}
			break;
		case INIT_DASH:
			curr->low = 1;

			if (isdigit(*list)) {
				state = DASH_NUM;
			} else {
				usage(EXIT_FAILURE);
			}
			break;
		case INIT_NUM:
			curr->low = strtol(list, &list, 10);
			curr->high = curr->low;

			if (*list == '-') {
				list++;
				state = NUM_DASH;
			} else if (*list == ',') {
				list++;	
				state = INITIAL;
			} else if (*list == '\0') {
				state = END;
			} else {
				usage(EXIT_FAILURE);
			}
			break;
		case DASH_NUM:
			curr->high = strtol(list, &list, 10);
			if (*list == ',') {
				state = INITIAL;	
				list++;
			} else if (*list == '\0') {
				state = END;
			} else {
				usage(EXIT_FAILURE);
			}
			break;
		case NUM_DASH:
			curr->high = LONG_MAX;

			if (*list == ',') {
				list++;
				state = INITIAL;
			} else if (isdigit(*list)) {
				state = NUM_DASH_NUM;
			} else if (*list == '\0') {
				state = END;
			} else {
				usage(EXIT_FAILURE);
			}
			break;
		case NUM_DASH_NUM:
			curr->high = strtol(list, &list, 10);
			if (curr->high < curr->low) {
				fputs("Range should be increasing\n", stderr);
				usage(EXIT_FAILURE);
			}

			if (*list == ',') {
				list++;
				state = INITIAL;
			} else if (*list == '\0') {
				state = END;
			} else {
				usage(EXIT_FAILURE);
			}
			break;
		case END:
			cont = 0;
			break;
		default:
			usage(EXIT_FAILURE);
		}
	}
}

/* TODO -This is very inefficient as it iterates over the linked list for every
 * byte in the files */
int in_range(int indx)
{
	struct range *tmp;

	tmp = ranges;
	indx++;		// Indexes start from 0, while ranges start from 1

	while (tmp != NULL) {
		if (tmp->low <= indx && tmp->high >= indx) {
			return 1 ^ complement;
		}
		tmp = tmp->next;
	}

	return complement;	// 0 ^ complement is same as complement
}

void bcut(FILE *f)
{
	int j;
	int indx;
	int btread;	/* number of bytes read from the file */
	char *buf;
	int prev_out;	/* 1 - previous token was outputed, 0 - was not*/
	int first;	/* It's the first token of the line */

	buf = malloc(sizeof(char) * 1024);
	indx = 0;
	prev_out = 0;
	first = 1;

	while ((btread = fread(buf, 1, BUF_SIZE, f)) != 0) {
		for (j = 0; j < btread; j++) {
			if (indx == 0) {
				first = 1;
				prev_out = 0;
			}

			if (buf[j] == '\n') {
				fputc(zero_terminate ? '\0' : '\n', 
					stdout);
				indx = 0;
			} else if (in_range(indx)) {
				if (prev_out == 0 && !first) {
					printf("%s", output_delimiter);
				}
				prev_out = 1;
				first = 0;
				fputc(buf[j], stdout);
				indx++;
			} else {
				indx++;
				prev_out = 0;
			}
		}
	}

	free(buf);

}

void fcut(FILE *f)
{
	int i;
	int indx;
	int output;	/*bool value, 1 output char, 0 do not */
	int btread;
	char *buf;
	int prev_out;	/* 1 - previous token was outputed, 0 - was not*/
	int first;	/* It's the first token of the line */

	buf = malloc(sizeof(char) * BUF_SIZE);
	indx = 0;
	prev_out = 0;
	first = 1;
	output = in_range(indx);

	while ((btread = fread(buf, 1, BUF_SIZE, f)) != 0) {
		for (i = 0; i < btread; i++) {
			if (indx == 0) {
				first = 1;
				prev_out = 0;
			}
			if (buf[i] == '\n') {
				fputc(zero_terminate ? '\0' : '\n', 
					stdout);
				indx = 0;
				output = in_range(indx);
			} else if (buf[i] == field_delimiter) {
				indx++;
				output = in_range(indx);
				fprintf(stderr, "DEL - >po %d, out %d\n",
				prev_out, output);
				if (prev_out && output)
					fputc(buf[i], stdout);
			} else if (output) {
				fputc(buf[i], stdout);
				if (prev_out == 0 && !first) {
					printf("%s", output_delimiter);
				}
				prev_out = 1;
			} else {
				prev_out = 0;
			}
		}
	}
	
}

void do_cut(enum operation_mode mode, char *argv[], int size)
{
	int i;
	FILE *f;


	for (i = 0; i < size; i++) {
		if ((f = fopen(argv[i], "r")) == NULL) {
			fprintf(stderr, "couldn't open the file %s\n", argv[i]);
		} else if (mode == bflag) {
			bcut(f);
		} else if (mode == fflag) {
			fcut(f);
		}
	}

}

int main(int argc, char *argv[])
{
	int ch;
	char *list_range;
	enum operation_mode mode = none;

	output_delimiter = NULL;
	setlocale(LC_ALL, "");

	while ((ch = getopt_long(argc, argv, "b:c:f:d:nsz", long_opt, NULL)) != -1) {
		switch(ch) {
		case 'b':
		case 'c':	// -b and -c options are the same
			if(mode != none)
				usage(EXIT_FAILURE);
			mode = bflag;
			list_range = optarg;
			break;
		case 'f':
			if(mode != none)
				usage(EXIT_FAILURE);
			mode = fflag;
			list_range = optarg;
			break;
		case 'd':
			/*TODO - add multibyte delimiter later */
			if (strlen(optarg) != 1)
				usage(EXIT_FAILURE);
			field_delimiter = *optarg;
			break;
		case 'n':
			/* Ignore this option */
			break;
		case 'm':
			complement = 1;
			break;
		case 's':
			break;
		case 'o':
			output_delimiter = optarg;
			break;
		case 'z':
			zero_terminate = 1;
			break;
			
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'v':
			puts("VERSION HERE");
			exit(EXIT_SUCCESS);
		default:
			usage(EXIT_FAILURE);
		}
	}

	if (mode != bflag && mode != fflag) 
		usage(EXIT_FAILURE);
	
	if (optind == argc)
		usage(EXIT_FAILURE);

	if (mode == fflag && !output_delimiter) {
		char del[2] = { field_delimiter, '\0'};
		output_delimiter = del;
	}

	initialize_ranges(list_range);
	do_cut(mode, argv + optind, argc - optind);

	exit(EXIT_SUCCESS);
}
