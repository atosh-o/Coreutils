/* There are couple of  ways of handling the *List* provided by the user

  FreeBSD style - We create a character buffer the size of the highest upper
  bound provided by the user in the _List_ and each nth byte represents whether
  nth token is included in the output or not

*/
//C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

//POSIX
#include <unistd.h>

//GNU
#include <getopt.h>

//Definitions
#define BUF_SIZE 1024
#define HIGH 0
#define LOW 1


struct option long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {"output-delimiter", required_argument, NULL, 'o'},
  {NULL, 0, NULL, 0}
};


static int list_size;
static char *output_delimiter;

int **parse_list_ranges(char *list_range){
  char *tmp;
  int i;
  int **list_arr;

  /* Count the number of commas, number of ranges is commas + 1 */
  for(tmp = list_range; *tmp != '\0'; tmp++){
    if(*tmp == ','){
      list_size++;
    }
  }
  list_size++; // One more option, than the comma

  /* Initialize Array */
  list_arr = malloc(sizeof(int) * list_size); //TODO - error checking
  for(i = 0; i < list_size; i++){
    list_arr[i] = malloc(sizeof(int) * 2); // Two places for start - stop
  }

  /*LESSON - strsep is BSD extension and not portable
    TODO - List parsing can be implemented as an FSA
  */

  for(i = 0;(tmp = strsep(&list_range, ",")) != NULL; i++){
    if(*tmp == '-'){
      list_arr[i][HIGH] = 0; 
      list_arr[i][LOW] = strtol(tmp + 1, &tmp, 10) - 1;
      continue;
    }
    if(isdigit(*tmp)){
     list_arr[i][HIGH] = list_arr[i][LOW] = strtol(tmp, &tmp, 10) - 1; 
    }
    if(*tmp == '-'){
      if(isdigit(*(tmp + 1))){
        list_arr[i][LOW] = strtol(tmp + 1, &tmp, 10) - 1;
      } else {
        list_arr[i][LOW] = INT_MAX;
      }
    }
  }
  return list_arr;

}

int in_range(int **list, int index){
  int i;
  static int *curr = NULL;

  if( curr && index >= curr[LOW] && index <= curr[HIGH]){
    return 1;
  } else {
    curr = NULL;
    for(i = 0; i < list_size; i++){
      if(index >= list[i][HIGH] && index <= list[i][LOW]){
        curr = list[i];
        return 1;
      }
    }
  }
  return 0;
}

void do_cut(int **list, char **argv, int argc){
  int     i;
  int     bytes_read;
  char *  buf;
  int     index;          //Index into the line
  int     use_delim;     //Track the delimiter between tokens


  buf = malloc(sizeof(char) * BUF_SIZE);

  for(i = 0; i < argc; i++){
    FILE *f = fopen(argv[i], "r");
    if(f == NULL){
      fprintf(stderr, "Usage: filename error\n");
      exit(EXIT_FAILURE);
    }

    index = 0;
    use_delim = 0;   //Do not use delimiter at the beginning

    while((bytes_read = fread(buf, 1, BUF_SIZE, f)) != 0){ //read 1024 bytes
      for(i = 0; i < bytes_read; i++){
        if(buf[i] == '\n'){
          index = 0;
          printf("\n");
          continue; // We don't want index incremented
        } else if(in_range(list, index)){
          printf("%c", buf[i]);
          use_delim = 1;
        } else if(use_delim){
          fputs(" ", stdout);
          use_delim = 0;
        }
      index++;
      }
    }

    
  }
}

int main(int argc, char *argv[]){

  #define BFLAG 1
  #define CFLAG 2
  #define FFLAG 3
  char      ch;
  int       select_flag;

  char  *   list_ranges;
  int   **  list;

  select_flag = 0;

  while((ch = getopt_long(argc, argv, "b:c:f:", long_options, NULL)) != -1){
    switch(ch){
      case 'b': // -b and -c are the same options.
      case 'c':
       if(select_flag){
        fprintf(stderr, "USage: double flags\n");
        exit(EXIT_FAILURE);
       }
       select_flag = BFLAG;
       list_ranges = optarg;
       break;
      case 'f':
       if(select_flag){
        fprintf(stderr, "USage: double flags\n");
        exit(EXIT_FAILURE);
       }
       select_flag = FFLAG;
       list_ranges = optarg;
       break;
      case 'o':
        output_delimiter = optarg;
        break;
        
      default:
        fprintf(stderr, "I get fired %d\n", optind);
        fprintf(stderr, "Usage: cut ..\n");
        exit(EXIT_SUCCESS);
    }
  }

  if(optind == argc){
    fprintf(stderr, "Usage: Need file name\n");
    exit(EXIT_FAILURE);
  }

  list = parse_list_ranges(list_ranges);

/*
  for(int i = 0; i < list_size; i++){
    fprintf(stderr, "start -> %d, stop -> %d\n", list[i][0], list[i][1]);
  }

*/
  do_cut(list, argv + optind, argc - optind);




}

