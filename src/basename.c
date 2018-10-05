//C
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

//POSIX
#include <unistd.h>

//GNU
#include <getopt.h>

//Definitions
#define PROGRAM_VERSION 0.1
#define initialize_main(ac, av) 
#define set_program_name(av) 


struct option long_options[] = {
  {"multiple", no_argument, NULL, 'a'},
  {"suffix", required_argument, NULL, 's'},
  {"zero", no_argument, NULL, 'z'},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {0, 0, 0, 0}
};

void
emit_try_help(){
  puts("try --help");
  exit(EXIT_FAILURE);
}

void
print_basedir(char *str, char *suff, int zero){
  char *basename = strrchr(str, '/');
  if(basename == NULL){
    basename = str;
  }
  if(suff != NULL){
    char *tmp = strrchr(basename, suff[0]);
    if(tmp != NULL && strcmp(tmp, suff) == 0){ //Lazy evaluation
      *tmp = '\0';
    }
  }
  printf("%s%c", basename, zero ? '\0' : '\n');
}

void 
usage (int status){
  if(status == EXIT_SUCCESS){
    emit_try_help();
  } else {
    printf("failure usage\n");
    exit(EXIT_FAILURE);
  }


}


int main(int argc, char *argv[]){
  int ch; // char unsigned by default, we need to check -1
  int multiple = 0;
  char *suffix = NULL;
  int zero = 0;

	initialize_main(&argc, &argv);
	set_program_name(argv[0]);
	setlocale(LC_ALL, "");

  while((ch = getopt_long(argc, argv, "+azs:", long_options, NULL)) != -1){
    switch(ch){
      case 'z':
        zero = 1;
        break;
      case 's':
        suffix = optarg;
        //fallthrough, s implies
      case 'a':
        multiple = 1;
        break;
      case 'v':
        printf("%s - Version %f\n", argv[0], PROGRAM_VERSION);
        exit(EXIT_SUCCESS);
      case 'h':
        usage(EXIT_SUCCESS);
      default:
        usage(EXIT_FAILURE);

    }
  }

 
  /*
  * We should check with optind. Even in case there were zero
  * option flags, user might have written out "--" before first argument.
  */
  if(argc == optind){
    puts("missing argument");
    exit(EXIT_FAILURE);
  }
  if(!multiple && argc > optind + 2){
    puts("extra operand\n");
    usage(EXIT_FAILURE);
  }

  if(!multiple){
    if(argc == optind + 2){
      print_basedir(argv[optind], argv[optind + 1], 0);
    } else {
      print_basedir(argv[optind], suffix,  zero);
    }
  } else {
    for(;optind != argc; optind++){
      print_basedir(argv[optind], suffix, zero);
    }
  }



}

/* Lessons Learned:
 * 1. Even in the programs that do not take any options, we can't expect
 * arguments to start at argv[1], because the user might enter "--"  before the
 * first argument, which is a behaviour that we should take in consideration.
 *
 *2. optind == argc implies that after parsing options there are no more
 *arguments left.
 *
 *3. strcmp Segfaults if provided with the NULL pointer.
 */
