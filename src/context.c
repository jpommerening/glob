#define _IN_GLOB_
#include "glob.h"
#include "internal.h"
#include <fnmatch.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

void glob_context_init( glob_context_t* context, fnmatch_pattern_t* pattern, const char* path ) {
  fnmatch_context_init( &(context->fnmatch), pattern );
  context->path = strdup( path );
}

void glob_context_destroy( glob_context_t* context ) {
  fnmatch_context_destroy( &(context->fnmatch) );
  free( context->path );
}

glob_state_t glob_context_match( glob_context_t* context ) {
  DIR* dir = opendir( context->path );
  struct dirent* dirent = readdir( dir );
  fnmatch_state_t state;

  if( dirent == NULL ) return GLOB_STOP;

  fnmatch_context_push( &(context->fnmatch), dirent->d_name );

  do {
    state = fnmatch_context_match( &(context->fnmatch) );
    switch( state ) {
      case FNMATCH_PUSH:
        /* recursive */
        break;
      case FNMATCH_POP:
        fnmatch_context_pop( &(context->fnmatch) );
        break;
      case FNMATCH_ERROR:
        return GLOB_ERROR;
      default:
        break;
    }
  } while( state != FNMATCH_STOP );

  return GLOB_STOP;
}
