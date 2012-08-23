#include "glob.h"
#include "test.h"

/* Stack:
 *
 * readdir: \0
 * dirent:  \0src/\0
 * dirent:  \0src/\0test/\0
 * dirent:  \0src/\0test/\0x.c\0
 * nomatch  \0src/\0test/\0
 * match:   \0src/\0
 * readdir: \0src/\0\0
 * dirent:  \0src/\0\0a.c\0
 * dirent:  \0src/\0\0a.c\0a.out\0
 * dirent:  \0src/\0\0a.c\0a.out\0a.h\0
 * match:   \0src/\0\0a.c\0a.out\0
 * nomatch: \0src/\0\0a.c\0
 * match:   \0src/\0\0
 * nomatch: \0
 */

TEST( test_context_simple ) {
  fnmatch_pattern_t pattern;
  glob_context_t context;
  
  fnmatch_pattern_init(&pattern);
  fnmatch_pattern_compile(&pattern, "test/*.[ch]");
  glob_context_init(&context, &pattern, "/root/");

  ASSERTEQ( GLOB_READDIR, glob_context_match(&context) );
  glob_context_dirent(&context, GLOB_DIR, "src/");
  glob_context_dirent(&context, GLOB_DIR, "test/");
  glob_context_dirent(&context, GLOB_FILE, "x.c");
  ASSERTEQ( GLOB_NOMATCH, glob_context_match(&context) ); /* /root/x.c */
  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );   /* /root/test/ */
  ASSERTEQ( GLOB_READDIR, glob_context_match(&context) ); /* /root/test/ */
  glob_context_dirent(&context, GLOB_FILE, "a.c");
  glob_context_dirent(&context, GLOB_FILE, "a.out");
  glob_context_dirent(&context, GLOB_FILE, "a.h");
  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );   /* /root/test/a.h */
  ASSERTEQ( GLOB_NOMATCH, glob_context_match(&context) ); /* /root/test/a.out */
  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );   /* /root/test/a.c */
/*ASSERTEQ( GLOB_POPDIR, glob_context_match(&context) );   * /root/test/ */
  ASSERTEQ( GLOB_NOMATCH, glob_context_match(&context) ); /* /root/src/ */
  ASSERTEQ( GLOB_STOP, glob_context_match(&context) );
  
  glob_context_destroy(&context);
  fnmatch_pattern_destroy(&pattern);
}

TEST( test_context_recursive ) {

}
