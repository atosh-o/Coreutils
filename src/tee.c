/* TODO - implement -p --output-error options. Need to read up on pipes and
 * signals before continuing.
*/

//C
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

//POSIX
#include <unistd.h>
#include <signal.h>

//GNU
#include <getopt.h>

//FAKE DEFINITIONS
#ifndef INIT_PROG
#define initialize_program(av, ac)
#endif

#ifndef SET_PROG_NAME
#define set_program_name(av) 
#endif

//DEFINITIONS
#define VERSION 0.1
#define BUF_SIZE 4096

struct option long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {"output-error", required_argument, NULL, 'o'},
  {"append", no_argument, NULL, 'a'},
  {"ignore-interrupts", no_argument, NULL, 'i'}
};

struct descriptors {
  FILE *file;
  struct descriptors *next;
};


enum mode {
  MODE_WARN,
  MODE_WARN_NOPIPE,
  MODE_EXIT,
  MODE_EXIT_NOPIPE
};

void usage(int status){
  if(status != EXIT_SUCCESS){
    fputs("Incorrect Usage: try --help\n", stderr);
  } else {
    fputs("Usage of this program - TBD\n", stdout); //TODO -
  }
  exit(status);
}


int main(int argc, char *argv[]){
	int append = 0;
	int interr = 0;
	int diagnose = 0;
  enum mode m;

	initialize_program(argc, argv);
	set_program_name(argv[0]);
	setlocale(LC_ALL, "");
	// TODO - textdomain stuff - implement later

  int c;
  opterr = 0;
  while((c = getopt_long(argc, argv, "api", long_options, NULL)) != -1) {
    switch(c) {
      case 'a':
        append = 1;
        break;

      case 'i':
        interr = 1;
        break;

      case 'p':
        diagnose = 1;
        break;

      case 'h':
        usage(EXIT_SUCCESS);
      case 'o':
        if(strcmp(optarg, "warn") == 0){
          m = MODE_WARN;
        } else if(strcmp(optarg, "warn-nopipe") == 0){
          m = MODE_WARN_NOPIPE;
        } else if(strcmp(optarg, "exit") == 0){
          m = MODE_EXIT;
        } else if(strcmp(optarg, "exit-nopipe") == 0){
          m = MODE_EXIT_NOPIPE;
        } else {
          usage(EXIT_FAILURE);
        }
        break;
      case 'v':
        fprintf(stdout, "Program version - %f\n", VERSION);
        exit(EXIT_SUCCESS);
      default:
        usage(EXIT_FAILURE);
    }
  }

  printf("mode - %d, append - %d, interr - %d, diagnose - %d\n", m,  append,
                                                      interr, diagnose);
  /* TODO -- THIS IS A MISTAKE, THIS BEHAVIOUR DOES NOT CONFORM WITH 
   * POSIX STANDARD,  WHEN GIVEN  NO ARGUMENT TEE SHOULD WORK NEVERTHELESS*/
  if(optind == argc){
    fputs("Missing arguments\n", stderr);
  }


  struct descriptors *first = malloc(sizeof(struct descriptors));
  first->file = stdin;
  first->next = NULL;


  /* set up the linked list of descriptors. first record is the stdout */
  struct descriptors *current = first;
  for(; optind < argc; optind++){
    FILE *current_file;
    if((current_file = fopen(argv[optind], append ? "a" : "w")) != NULL){
      current->next = malloc(sizeof(struct descriptors));
      current = current->next;
      current->file = current_file;
      current->next = NULL;
    }
  }


  char *buf = malloc(sizeof(char) * BUF_SIZE);
  int bytesRead;

  /* read stdin until EOF */
  while((bytesRead = read(STDIN_FILENO, buf, BUF_SIZE)) != 0){
    for(current = first; current != NULL; current = current->next){
      fwrite(buf, bytesRead, 1, current->file);    
    }
  }



  exit(EXIT_SUCCESS);


}
