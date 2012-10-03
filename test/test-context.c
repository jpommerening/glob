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
  glob_context_dirent(&context, "src/");
  glob_context_dirent(&context, "test"); /* must work with and without trailing slash */
  glob_context_dirent(&context, "x.c");

  ASSERTEQ( GLOB_NOMATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/x.c", context.path );

  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/test", context.path );

  ASSERTEQ( GLOB_READDIR, glob_context_match(&context) );
  glob_context_dirent(&context, "a.c");
  glob_context_dirent(&context, "a.out");
  glob_context_dirent(&context, "a.h");

  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/test/a.h", context.path );

  ASSERTEQ( GLOB_NOMATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/test/a.out", context.path );

  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/test/a.c", context.path );

  ASSERTEQ( GLOB_NOMATCH, glob_context_match(&context) );
  ASSERTSTREQ( "/root/src", context.path );

  ASSERTEQ( GLOB_STOP, glob_context_match(&context) );
  
  glob_context_destroy(&context);
  fnmatch_pattern_destroy(&pattern);
}

TEST( test_context_recursive ) {
  
}
