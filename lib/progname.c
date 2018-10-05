#include <errno.h> // get program_invocation_name

char *program_name;

void set_program_name(char *naked){
  /* Real coreutils strips argv0 in case where it is preceded by the .libs
   * directory or is prefexed by lt- <somepath>/.libs/lt-tee, would be striped
   * to tee. We won't do anything similar in this function
   */

   program_name = naked;
   program_invocation_name = naked; // Used by errno.h functions.

}
