#include "glob.h"
#include "test.h"

/**
 * This is a quick matching test, assuring that basic matching
 * works and the directory stack is traversed correctly.
 *
 * CALL    RETURN   STACK                       PATH
 * ------  -------  --------------------------  ------------------
 * match   readdir  \0                          /root/
 * dirent  -        src/\0                      /root/
 * dirent  -        src/\0test/\0               /root/
 * dirent  -        src/\0test/\0x.c\0          /root/
 * match   nomatch  src/\0test/\0               /root/x.c
 * match   match    src/\0                      /root/test/
 * match   readdir  src/\0\0                    /root/test/
 * dirent  -        src/\0\0a.c\0               /root/test/
 * dirent  -        src/\0\0a.c\0a.out\0        /root/test/
 * dirent  -        src/\0\0a.c\0a.out\0a.h\0   /root/test/
 * match   match    src/\0\0a.c\0a.out\0        /root/test/a.h
 * match   nomatch  src/\0\0a.c\0               /root/test/a.out
 * match   match    src/\0\0                    /root/test/a.c
 * match   nomatch  \0                          /root/src/
 * ------  -------  --------------------------  ------------------
 */
TEST( test_context_simple ) {
  fnmatch_pattern_t pattern;
  glob_context_t context;
  
  fnmatch_pattern_init(&pattern);
  fnmatch_pattern_compile(&pattern, "test/*.[ch]");
  glob_context_init(&context, &pattern, "/root/");

  ASSERTEQ( GLOB_READDIR, glob_context_match(&context) );
  glob_context_dirent(&context, GLOB_DIR, "src/");
  glob_context_dirent(&context, GLOB_DIR, "test"); /* must work with and without trailing slash */
  glob_context_dirent(&context, GLOB_FILE, "x.c");

  ASSERTEQ( GLOB_NOMATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/x.c", context.path );
  ASSERTEQ( GLOB_FILE, context.type );

  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/test/", context.path );
  ASSERTEQ( GLOB_DIR, context.type );

  ASSERTEQ( GLOB_READDIR, glob_context_match(&context) );
  glob_context_dirent(&context, GLOB_FILE, "a.c");
  glob_context_dirent(&context, GLOB_FILE, "a.out");
  glob_context_dirent(&context, GLOB_FILE, "a.h");

  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/test/a.h", context.path );
  ASSERTEQ( GLOB_FILE, context.type );

  ASSERTEQ( GLOB_NOMATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/test/a.out", context.path );
  ASSERTEQ( GLOB_FILE, context.type );

  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/test/a.c", context.path );
  ASSERTEQ( GLOB_FILE, context.type );

  ASSERTEQ( GLOB_NOMATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/src/", context.path );
  ASSERTEQ( GLOB_DIR, context.type );

  ASSERTEQ( GLOB_STOP, glob_context_match(&context) );
  
  glob_context_destroy(&context);
  fnmatch_pattern_destroy(&pattern);
}

TEST( test_context_recursive ) {

}
