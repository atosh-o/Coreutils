/*
	[ ] 
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>

#include <getopt.h>

struct option long_opt[] = {
	{"changes", no_argument, NULL, 'c'},
	{"silent", no_argument, NULL, 'f'},
	{"quiet", no_argument, NULL, 'f'},
	{"dereference", no_argument, NULL, 'd'},
	{"from", optional_argument, NULL, 'F'},
	{"no-preserve-root", no_argument, NULL, 'n'},
	{"preserve-root", no_argument, NULL, 'p'},
	{"reference", required_argument, NULL, 'r'},
	{"recursive", no_argument, NULL, 'R'},
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'V'},
	{NULL, 0, NULL, 0}
};

enum recurs_mode {hflag, lflag, pflag};

struct chown_options {
	char *owner;
	char *group;
	char changes;
	char silent;
	char verbose;
	char dereference;
	char *from_owner;
	char *from_group;
	char preserve_root;
	char *ref_file;
	char recurs;
	enum recurs_mode rmode;

};

void usage(int s)
{
	exit(s);
}

/* Parse [OWNER][:[GROUP]] with only one iteration over the string
	- If parsing fails this function terminates the program with
corresponding error message
	- If the function returns either *user or *grp will have non NULL value
	
*/
/* TODO - move out this function to seperate file */
void parse_usrgrp(char *str, char **user, char **grp)
{
	int colon_count;

	*user = str;
	colon_count = 1;
	for (; *str != '\0'; str++) {
		if (*str == ':') {
			if (colon_count++ > 1) {
				fputs("Invalid Owner Group pair\n", stderr);
				usage(EXIT_FAILURE);
			} else {
				*str = '\0';
				*grp = str + 1;
			}
		}
	}

	if (strlen(*user) == 0)
		*user = NULL;
	if (strlen(*grp) == 0)
		*grp = NULL;
	
	if (*user == NULL && *grp == NULL) {
		fputs("Invalid Owner Group pair\n", stderr);
		usage(EXIT_FAILURE);
	}

}

int do_chown(int argc, char *argv[], struct chown_options *opts)
{
	FILE *f;
	int i;
	int ret;
	struct stat st;

	ret = EXIT_SUCCESS;

	/* Iterate over files */
	for (i = 0; i < argc; i++) {
		if (!opts->recurs) {
			file_chown(argv[i], opts->owner, opts->group);
		}
	}
	return ret;
}

int main(int argc, char *argv[])
{
	int ch;
	char *tmp;
	int colon_count;
	struct chown_options *opts;

	opts = malloc(sizeof(struct chown_options));
	memset(opts, 0, sizeof(struct chown_options));
	opts->rmode = pflag;

	while ((ch = getopt_long(argc, argv, "cfvhR", long_opt, NULL)) != -1) {
		switch (ch) {
		case 'c':
			opts->changes = 1;
			break;
		case 'f':
			opts->silent = 1;
			break;
		case 'v':
			opts->verbose = 1;
			break;
		case 'd':
			opts->dereference = 1;
			break;
		case 'F':
			parse_usrgrp(optarg, &opts->from_owner, &opts->from_group);
			break;
		case 'n':
			/* opts->preserver_root is 0 by default */
			break;
		case 'p':
			opts->preserve_root = 1;
			break;
		case 'r':
			opts->ref_file = optarg;
			break;
		case 'R':
			opts->recurs = 1;
			break;
		case 'H':
			opts->rmode = hflag;
			break;
		case 'L':
			opts->rmode = lflag;
			break;
		case 'P':
			opts->rmode = pflag;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'V':
			puts("version");
			exit(EXIT_SUCCESS);
		
		}
	}
	
	argv += optind;
	argc -= optind;

	if (opts->ref_file == NULL && argc != 0) {
		parse_usrgrp(*argv, &opts->owner, &opts->group);
		argv++;
		argc--;
	}


	if (argc == 0) {
		fputs("You must provide file\n", stderr);
		usage(EXIT_FAILURE);
	}

	exit(do_chown(argc, argv, opts));
	
}
