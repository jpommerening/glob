#define _IN_GLOB_
#include "glob.h"
#include "internal.h"
#include <fnmatch.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>

void glob_context_init( glob_context_t* context, fnmatch_pattern_t* pattern, const char* path ) {
  size_t length = strlen( path ) + 1;
  fnmatch_context_init( &(context->fnmatch), pattern );
  GLOB_ALLOC(context->path, length, &(context->palloc));
  memcpy(context->path, path, length);
  context->plen = length-1;
  
  context->buffer = NULL;
  context->buflen = 0;
  context->bufalloc = 0;

  context->type  = GLOB_END;
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
  size_t length;
  char*  bufend;
  char*  dirent;

  if( context->buflen == 0 ) {
    printf( "Buffer empty!\n" );
  }

  /* let bufend point to terminating \0 */
  bufend = &(context->buffer[context->buflen-1]);
  
  /* let dirent point to the first character of the last dir entry */
  for( dirent = bufend; dirent > context->buffer && *(dirent-1) != '\0'; dirent-- );

  /* length of the entry excluding the terminating \0 */
  length = bufend - dirent;

  if( length > 0 ) {
    printf( "Consume dirent %s in directory %s (bufend: %p, dirent: %p)\n", dirent, context->path, bufend, dirent );

    GLOB_GROW(context->path, context->plen+length+1, &(context->palloc));
    memcpy( &(context->path[context->plen]), dirent, length+1 );
    context->plen += length;

    context->buflen -= length+1;
    fnmatch_context_push( &(context->fnmatch), dirent );
  } else {
    printf( "Finished dir %s (bufend: %p, dirent: %p)\n", context->path, bufend, dirent );

    context->buflen -= 1;
    fnmatch_context_push( &(context->fnmatch), NULL );
  }
}

/**
 * Pop an item from the fnmatch context.
 * Update the path and filetype properties.
 */
static glob_state_t glob__context_pop( glob_context_t* context ) {
  size_t offset = context->plen;
  const char* str = fnmatch_context_pop( &(context->fnmatch) );
  
  printf( "pop %s\n", str );
  while( offset>0 && context->path[offset-1] != '/' ) --offset;
  assert( strcmp( str, &(context->path[offset]) ) == 0 && "fnmatch_context_pop returned unexpected entry" );
  context->path[offset] = '\0';
  context->plen = offset;
  
  context->type = GLOB_DIR;
  
  printf( "Pop: %s, Path: %s\n", str, context->path );

  return GLOB_CONTINUE;
}

void glob_context_dirent( glob_context_t* context, glob_filetype_t type, const char* dirent ) {
  size_t length = strlen( dirent );
  size_t offset = context->buflen;
  /* reserve place for the directory entry, an optional trailing slash and the terminating \0 */
  GLOB_GROW(context->buffer, offset+length+2, &(context->bufalloc));
  memcpy(&(context->buffer[offset]), dirent, length);
  
  if( type == GLOB_DIR && dirent[length-1] != '/' ) {
    /* assure there's a trailing slash on all directories */
    context->buffer[offset+length] = '/';
    context->buffer[offset+length+1] = '\0';
    context->buflen = offset + length + 2;
  } else {
    /* append \0 on directories with trailing slash and plain files */
    context->buffer[offset+length] = '\0';
    context->buflen = offset + length + 1;
  }
  printf( "Added dirent %s in %s\n", &(context->buffer[offset]), context->path );
}

glob_state_t glob_context_match( glob_context_t* context ) {
  fnmatch_state_t state = context->fnmatch.state;

  if( context->buflen == 0 )
    return GLOB_READDIR;

  do {
    switch( state ) {
      case FNMATCH_POP:
        printf( "glob ctx pop\n" );
        glob__context_pop( context );
        break;
      case FNMATCH_PUSH:
        printf( "glob ctx push\n" );
        glob__context_push( context );
        break;

      case FNMATCH_NOMATCH:
        printf( "fnm nomatch (%s)\n", context->fnmatch.buffer );
        fnmatch_context_match( &(context->fnmatch) );
        return GLOB_NOMATCH;
      case FNMATCH_MATCH:
        printf( "fnm match (%s)\n", context->fnmatch.buffer );
        fnmatch_context_match( &(context->fnmatch) );
        return GLOB_MATCH;

      case FNMATCH_STOP:
        printf( "fnm stop\n" );
        return GLOB_STOP;
      case FNMATCH_ERROR:
        printf( "fnm error\n" );
        return GLOB_ERROR;
    }

    state = fnmatch_context_match( &(context->fnmatch) );

  } while( state != FNMATCH_STOP );

  printf( "glob stop\n" );
  return GLOB_STOP;
}
