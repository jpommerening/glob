#define _IN_GLOB_
#include "glob.h"
#include "internal.h"
#include <fnmatch.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * Stack layout
 *
 * (breadth first -- columns before rows)
 *
 * root\0path0\0
 *       path1\0file0\0
 *              file1\0
 *              file2\0
 *              \0
 *       path2\0
 *       \0
 * \0
 *
 * track:
 * [0] *root
 * [1] *path1
 * [2] *file2
 */

void glob_context_init( glob_context_t* context, fnmatch_pattern_t* pattern, const char* path ) {
  size_t length = strlen( path ) + 1;
  fnmatch_context_init( &(context->fnmatch), pattern );
  GLOB_ALLOC(context->path, length, &(context->palloc));
  memcpy(context->path, path, length);
  context->plen = length-1;
  
  context->buffer = NULL;
  context->buflen = 0;
  context->bufalloc = 0;
}

void glob_context_destroy( glob_context_t* context ) {
  fnmatch_context_destroy( &(context->fnmatch) );
  free( context->buffer );
}

/**
 * Take one entry from the directory heap, push it to the fnmatch context.
 * Update the path and filetype properties with the current path.
 */
static void glob__context_push( glob_context_t* context ) {
  /* remove from stack, push to fnmatch context & path */
  size_t offset = context->buflen-1;
  size_t length = 0;
  
  if( context->buflen == 0 ) {
    fnmatch_context_push( &(context->fnmatch), NULL );
    return;
  }
  assert( context->buffer[offset] == '\0' );
  
  while( offset>0 && context->buffer[offset-1] != '\0' ) --offset;

  context->type = context->buffer[offset];
  length = context->buflen - offset - 1;
  
  GLOB_GROW(context->path, context->plen+length, &(context->palloc));
  memcpy(&(context->path[context->plen]), &(context->buffer[offset+1]), length);
  context->plen += length-1;
    
  context->buflen = offset;
  fnmatch_context_push( &(context->fnmatch), &(context->buffer[offset+1]) );

  if( context->type == GLOB_FILE ) {
    fnmatch_context_push( &(context->fnmatch), NULL );
  }
  
  /*printf( "Push: %s, Path: %s\n", &(context->buffer[offset+1]), context->path );*/
}

/**
 * Pop an item from the fnmatch context.
 * Update the path and filetype properties.
 */
static void glob__context_pop( glob_context_t* context ) {
  size_t offset = context->plen;
  const char* str = fnmatch_context_pop( &(context->fnmatch) );
  
  while( offset>0 && context->path[offset-1] != '/' ) --offset;
  assert( strcmp( str, &(context->path[offset]) ) == 0 && "fnmatch_context_pop returned unexpected entry" );
  context->path[offset] = '\0';
  context->plen = offset;
  
  context->type = GLOB_DIR;
  
  /*printf( "Pop: %s, Path: %s\n", str, context->path );*/
}

void glob_context_dirent( glob_context_t* context, glob_filetype_t type, const char* dirent ) {
  size_t length = strlen( dirent ) + 2;
  size_t offset = context->buflen;
  GLOB_GROW(context->buffer, offset+length, &(context->bufalloc));
  memcpy(&(context->buffer[offset+1]), dirent, length);
  context->buffer[offset] = type;
  context->buflen += length;
  
  if( type == GLOB_DIR && dirent[length-2] != '/' ) {
    GLOB_GROW(context->buffer, offset+length+1, &(context->bufalloc));
    context->buffer[offset+length] = '/';
    context->buffer[offset+length] = '\0';
  }
}

glob_state_t glob_context_match( glob_context_t* context ) {
  fnmatch_state_t state;

  if( context->buflen == 0 )
    return GLOB_READDIR;
  
  do {
    state = fnmatch_context_match( &(context->fnmatch) );
    switch( state ) {
      case FNMATCH_MATCH:
        return GLOB_MATCH;
      case FNMATCH_NOMATCH:
        return GLOB_NOMATCH;
      case FNMATCH_PUSH:
        glob__context_push( context );
        break;
      case FNMATCH_POP:
        glob__context_pop( context );
        break;
      case FNMATCH_ERROR:
        return GLOB_ERROR;
      default:
        break;
    }
  } while( state != FNMATCH_STOP );

  return GLOB_STOP;
}
