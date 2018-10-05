/*
  GNU version of echo is not POSIX compliant. POSIX mandates that echo should
  take no options, it should write given arguments to the standard output and
  append newline at the end. GNU version takes options to control the
  interpretation of special characters and to opt out from newline at the end.

*/
//C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//POSIX
#include <unistd.h>

//GNU
#include <getopt.h>

//Definitions
#define VERSION 0.1
#define NEWLINE 10

//Fake Definitions, so It looks like I'm a coreutils developer
#define initialize_main(ac, av) 
#define set_program_name(ag) 

struct option long_options[] = {
  {"help", no_argument, NULL, 'h'},	
  {"version", no_argument, NULL, 'v'}
};

void
usage(int status){
  if(status == EXIT_FAILURE){
    fprintf(stderr, "Usage with failure\n");
  } else {
    printf("Usage succedded\n");
  }
  exit(status);
}

void
interpr_print(char *str){
  /* I've rewritten this part as FSA, but it became much more unreadable than it
   * was before the refactoring*/
#define STRING_STATE 0
#define BACKSLASH_STATE 1

  int state = STRING_STATE;
  for(; *str != '\0'; str++){

    switch(state){ /*Main state switch*/
      case STRING_STATE:
        if(*str == '\\'){
          state = BACKSLASH_STATE;
        } else {
          printf("%c", *str);
        }
        break;
      case BACKSLASH_STATE:

        switch(*str){ /* Backslash switch */
          case '\\':
            fputs("\\", stdout);
            state = STRING_STATE;
            break;
          case 'a':
            printf("%c", 7); //07 is the Bell character
            state = STRING_STATE;
            break;
          case 'b':
            printf("%c", 8); // Backspace
            state = STRING_STATE;
            break;
          case 'c':
            exit(EXIT_SUCCESS);
          case 'e':
            printf("%c", 27); //27 is Escape
            state = STRING_STATE;
            break;
          case 'f':
            printf("%c", 12); //form feed
            state = STRING_STATE;
            break;
          case 'n':
            fputs("\n", stdout);
            state = STRING_STATE;
            break;
          case 'r':
            fputs("\r", stdout);
            state = STRING_STATE;
            break;
          case 't':
            printf("%c", 9);
            state = STRING_STATE;
            break;
          case 'v':
            printf("%c", 11);
            state = STRING_STATE;
            break;
          default:
            usage(EXIT_FAILURE);
        }

        break;
    }
  }

  /*
  int control = 0;
  char ch;
  for(; *str != '\0'; str++){
    if(control){
      switch(*str){
        case '\\':
          ch = '\\';
          break;
        case 'n':
          ch = NEWLINE;
          break;
          
          
      }
      control = 0;
    } else {
      if(*str != '\\'){
        ch = *str;
      } else {
        control = 1;
      }
    }
    if(!control){
      printf("%c", ch);
    }
  }
  */
}

int main(int argc, char *argv[]){
  int newline = 1;
  int interpret = 0;
  int c;

  //parse long options without getopt_long
  if(argc == 2){
    if(strcmp(argv[1], "--help") == 0){
      usage(EXIT_SUCCESS);
    }
    if(strcmp(argv[1], "--version") == 0){
      printf("Program VERSION - %2f\n", VERSION);
      exit(EXIT_SUCCESS);
    }
  }

  int end_of_options = 0;
  opterr = 0;
  while((c = getopt(argc, argv, "+neE")) != -1 && !end_of_options){
    switch(c){
      case 'n':
        newline = 0;
        break;
      case 'e':
        interpret = 1;
        break;
      case 'E':
        interpret = 0;
        break;
      default:
        /* Break out of the while loop, every other argument should be
        interpreted as a literal string to be printed
        */
        end_of_options = 1;
    }
  }

  if(optind == argc){
    fputs("\n", stdout);
    exit(EXIT_SUCCESS);
  }

  // TODO - Ugly hack for spaces between the words, come back to it.
  int tmp = optind;

  for(; optind < argc ; optind++){
    if(!(tmp == optind)) {
      fputs(" ", stdout);
    }
    
    if(interpret){
      /* this function will interpret and print the string. It's better than
       * returning the string, as there is no need for buffer management. */
      interpr_print(argv[optind]);
    } else {
      fputs(argv[optind], stdout);
    }
  }

  if(newline){
    fputs("\n", stdout);
  }


  
}
