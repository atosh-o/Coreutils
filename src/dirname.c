//C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>


//POSIX
#include <unistd.h>

//GNU
#include <getopt.h>


//Real definitios
#define VERSION 0.1

//Fake definitions
/*
  define fake methods to mimic coreutils
*/
#define initialize_main(ac, av) 
#define set_program_name(name) 




struct option long_options[] = {
  {"zero", no_argument, NULL, 'z'},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'}
};

void try_help(){
  //TODO - rewrite 
  puts("return bad status and explain the program");
  exit(EXIT_FAILURE);
}

void
usage(int status){
  if(status != EXIT_SUCCESS){
    try_help();
  } else {
    //TODO - rewrite
    puts("Explain the program usage, status success");
  }
}

void
print_dirname(const char* dirname, int zero){
  //TODO - this is not POSIX compliant, fixit.
  char *last_slash;
  last_slash = strrchr(dirname, '/');
  if(last_slash != NULL){
    *last_slash = '\0';
    printf("%s%c", dirname, zero ? '\0' : '\n');
  } else {
    printf(".%c", zero ? '\0' : '\n');
  }

}

int 
main(int argc, char *argv[]){
  int c;
  int zero = 0;

  initialize_main(argc, argv);
  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);


  while((c = getopt_long(argc, argv, "+z", long_options, NULL)) != -1){
    switch(c){
      case 'z':
        zero = 1;
        break;
      case 'h':
        usage(EXIT_SUCCESS);
      case 'v':
        //TODO - Don't know whether I can rely on argv[0]
        printf("%s Version - %f\n", argv[0], VERSION);
        exit(EXIT_SUCCESS);
      default:
       exit(EXIT_FAILURE); 

    }
  }

  if(optind == argc){
    fprintf(stderr, "missing argument\n");
    usage(EXIT_FAILURE);
  }

  int i;
  for(i = optind; i < argc; i++){
    print_dirname(argv[i], zero);
  }
}


