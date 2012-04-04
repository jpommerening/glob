#define _IN_GLOB_
#include "glob.h"
#include "internal.h"
#include <fnmatch.h>
#include <dirent.h>

void glob_context_init( glob_context_t* context, fnmatch_pattern_t* pattern, const char* path ) {
  fnmatch_context_init( &(context->fnmatch), pattern );
  context->path = strdup( path );
}

void glob_context_destroy( glob_context_t* context ) {
  fnmatch_context_destroy( &(context->fnmatch) );
  free( context->path );
}

glob_state_t glob_context_match( glob_context_t* context ) {
  return GLOB_STOP;
}
