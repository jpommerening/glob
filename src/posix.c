#define _IN_GLOB_
#include "glob.h"
#include "internal.h"

int glob( const char* expr, int flags,
  int (*errfunc) (const char*, int), glob_t *glob ) {

  glob_state_t      state;
  glob_context_t    context;
  fnmatch_pattern_t pattern;
  char cwd[1024];

  getcwd( &(cwd[0]), sizeof(cwd) );

  fnmatch_pattern_init( &pattern );
  fnmatch_pattern_compile( &pattern, expr );

  glob_context_init( &context, &pattern, &(cwd[0]) );

  glob_context_destroy( &context );
  fnmatch_pattern_destroy( &pattern );

  return GLOB_NOSYS;
}

void globfree( glob_t *glob ) {
  /* :) */
}
