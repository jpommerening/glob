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
    if( context->path.length > 0 && context->path.data[context->path.length-1] != '/' ) {
      buffer_appendc( &(context->path), '/' );
    }
    buffer_append( &(context->path), dirent, length );
  }
}

static void glob__context_pushstack( glob_context_t* context, const char* str ) {
  size_t length = strlen(str) + 1;
  buffer_append( &(context->stack), str, length );
  context->dirent = &(context->stack.data[context->stack.length - length]);
}

static const char* glob__context_popstack( glob_context_t* context ) {
  size_t offset = context->dirent - context->stack.data;
  char*  dirent = context->dirent;
  assert( context->dirent != NULL );
  
  
  if( offset >= 1 ) {
    buffer_setlen( &(context->stack), offset - 1 );
    
    do {
      offset -= 1;
    } while( offset > 0 && context->stack.data[offset-1] != '\0' );
    
    context->dirent = &(context->stack.data[offset]);
  } else {
    buffer_setlen( &(context->stack), 0 );
    
    context->dirent = NULL;
  }
  
  return dirent;
}

static void glob__context_startstack( glob_context_t* context ) {
  buffer_appendc( &(context->stack), 0 );
  context->dirent = NULL;
}

static void glob__context_endstack( glob_context_t* context ) {
  buffer_setlen( &(context->stack), context->stack.length - 1 );
}

void glob_context_init( glob_context_t* context, fnmatch_pattern_t* pattern, const char* path ) {
  size_t length = strlen( path );
  
  buffer_init( &(context->path), length );
  buffer_init( &(context->stack), 0 );
  context->dirent = NULL;
  
  fnmatch_context_init( &(context->fnmatch), pattern );
  glob__context_path( context, path, length );
}

void glob_context_destroy( glob_context_t* context ) {
  fnmatch_context_destroy( &(context->fnmatch) );
  buffer_destroy( &(context->path) );
  buffer_destroy( &(context->stack) );
}

/**
 * Take one entry from the directory heap, push it to the fnmatch context.
 * Update the path and filetype properties with the current path.
 */
static void glob__context_push( glob_context_t* context ) {
  const char* dirent;

  if( context->stack.length == 0 ) {
    printf( "Buffer empty!\n" );
    return;
  }
  
  dirent = glob__context_popstack( context );
  
  if( *dirent == '\0' ) {
    printf( "push NULL\n" );
    fnmatch_context_push( &(context->fnmatch), NULL );
  } else {
    printf( "push dirent %s\n", dirent );
    glob__context_path( context, dirent, 0 );
    if( context->fnmatch.buffer.length > 0 &&
        context->fnmatch.buffer.data[context->fnmatch.buffer.length-1] != '/' )
      fnmatch_context_push( &(context->fnmatch), "/" );
    fnmatch_context_push( &(context->fnmatch), dirent );
  }
}

/**
 * Pop an item from the fnmatch context.
 * Update the path and filetype properties.
 */
static glob_state_t glob__context_pop( glob_context_t* context ) {
  size_t offset = context->path.length;
  const char* str = fnmatch_context_pop( &(context->fnmatch) );
  
  printf( "pop %s\n", str );
  while( offset>0 && context->path.data[offset] != '/' ) --offset;
  assert( strcmp( str, &(context->path.data[offset+1]) ) == 0 && "fnmatch_context_pop returned unexpected entry" );
  buffer_setlen( &(context->path), offset );
  
  printf( "Pop: %s, Path: %s\n", str, context->path.data );

  return GLOB_CONTINUE;
}

void glob_context_dirent( glob_context_t* context, const char* dirent ) {
  if( dirent == NULL || *dirent == '\0' ) {
    glob__context_endstack( context );
  } else {
    glob__context_pushstack( context, dirent );
    printf( "Added dirent %s in %s\n", dirent, context->path.data );
  }
}

glob_state_t glob_context_match( glob_context_t* context ) {
  fnmatch_state_t state = context->fnmatch.state;

  if( context->stack.length == 0 )
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
        printf( "fnm nomatch (%s)\n", context->fnmatch.buffer.data );
        fnmatch_context_match( &(context->fnmatch) );
        return GLOB_NOMATCH;
      case FNMATCH_MATCH:
        printf( "fnm match (%s)\n", context->fnmatch.buffer.data );
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
