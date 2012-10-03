#define _IN_GLOB_
#include "glob.h"
#include "internal.h"
#include <fnmatch.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>


static void glob__context_path( glob_context_t* context, const char* dirent, size_t length ) {
  if( length == 0 ) length = strlen( dirent );
  
  if( dirent != NULL ) {
    if( context->plen == 0 ) {
      GLOB_ALLOC(context->path, length + 1, &(context->palloc) );
    } else {
      GLOB_GROW(context->path, context->plen+length+2, &(context->palloc));
      context->path[context->plen] = '/';
      context->plen += 1;
    }
    
    memcpy( &(context->path[context->plen]), dirent, length );

    context->plen += length;
    context->path[context->plen] = '\0';
  }
}

static void glob__context_pushstack_( glob_context_t* context, const char* bytes, size_t len, size_t grow ) {
  assert( grow >= len );
  
  GLOB_GROW(context->buffer, context->buflen+grow, &(context->bufalloc));
  memcpy(&(context->buffer[context->buflen]), bytes, len);
  context->buflen += len;
}

static void glob__context_pushstack( glob_context_t* context, const char* str ) {
  size_t length = strlen(str) + 1;
  glob__context_pushstack_( context, str, length, length );
}

static void glob__context_startstack( glob_context_t* context ) {
  glob__context_pushstack_( context, "", 1, 16 );
  context->dirent = NULL;
}

static void glob__context_endstack( glob_context_t* context ) {
  glob__context_pushstack_( context, "", 1, 1 );
}

void glob_context_init( glob_context_t* context, fnmatch_pattern_t* pattern, const char* path ) {
  size_t length = strlen( path );
  if( length > 1 && path[length-1] == '/' ) length--;
  
  context->path = NULL;
  context->plen = 0;
  context->palloc = 0;
  
  context->buffer = NULL;
  context->buflen = 0;
  context->bufalloc = 0;
  context->dirent = NULL;
  
  fnmatch_context_init( &(context->fnmatch), pattern );
  glob__context_path( context, path, length );
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
    return;
  }
  /*
  dirent = context->dirent;
  if( dirent == NULL ) {
    printf( "Dirent NULL!\n" );
    fnmatch_context_push( &(context->fnmatch), NULL );
    
    context->buflen -= 1;
    dirent = &(context->buffer[context->buflen]);
  } else {
    printf( "Dirent %s\n", dirent );
    if( context->fnmatch.buflen > 0 )
      fnmatch_context_push( &(context->fnmatch), "/" );
    
    fnmatch_context_push( &(context->fnmatch), dirent );
    glob__context_path( context, dirent, 0 );
    dirent[0] = '\0';
    dirent -= 1;
    
    context->buflen = dirent - context->buffer;
  }
  
  * let dirent point to the first character of the last dir entry if any *
  while( dirent > context->buffer && *(dirent-1) != '\0' ) dirent--;
  if( dirent[0] == '\0' ) {
    context->dirent = NULL;
  } else {
    context->dirent = dirent;
  }
  
  return;
  */

  /* let bufend point to terminating \0 */
  bufend = &(context->buffer[context->buflen-1]);
  if( bufend > context->buffer && *(bufend-1) == '\0' )
    bufend--;
  
  /* let dirent point to the first character of the last dir entry */
  for( dirent = bufend; dirent > context->buffer && *(dirent-1) != '\0'; dirent-- );

  /* length of the entry excluding the terminating \0 */
  length = bufend - dirent;

  if( length > 0 ) {
    printf( "Consume dirent %s in directory %s (bufend: %p, dirent: %p)\n", dirent, context->path, bufend, dirent );

    context->buflen -= length;

    if( context->fnmatch.buflen > 0 )
      fnmatch_context_push( &(context->fnmatch), "/" );
    
    glob__context_path( context, dirent, length );
    fnmatch_context_push( &(context->fnmatch), dirent );
    *(dirent) = '\0';
  } else {
    printf( "Finished dir %s (bufend: %p, dirent: %p)\n", context->path, bufend, dirent );

    context->buflen -= 2;

    glob__context_path( context, dirent, 0 );
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
  while( offset>0 && context->path[offset] != '/' ) --offset;
  assert( strcmp( str, &(context->path[offset+1]) ) == 0 && "fnmatch_context_pop returned unexpected entry" );
  context->path[offset] = '\0';
  context->plen = offset;
  
  printf( "Pop: %s, Path: %s\n", str, context->path );

  return GLOB_CONTINUE;
}

void glob_context_dirent( glob_context_t* context, const char* dirent ) {
  size_t length = strlen( dirent );
  size_t offset = context->buflen;
  /* reserve place for the directory entry and the terminating \0 */
  GLOB_GROW(context->buffer, offset+length+1, &(context->bufalloc));
  memcpy(&(context->buffer[offset]), dirent, length);

  context->dirent = &(context->buffer[offset]);
  
  if( dirent[length-1] == '/' ) {
    /* assure there's a trailing slash on all directories */
    context->buffer[offset+length-1] = '\0';
    context->buflen += length;
  } else {
    context->buflen += length+1;
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
