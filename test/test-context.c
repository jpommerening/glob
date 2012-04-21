#include "glob.h"
#include "test.h"

TEST( test_context ) {
  fnmatch_pattern_t pattern;
  glob_context_t context;
  
  fnmatch_pattern_init(&pattern);
  fnmatch_pattern_compile(&pattern, "test/*.[ch]");
  glob_context_init(&context, &pattern, "/root/");
  
  ASSERTEQ( GLOB_READDIR, glob_context_match(&context) );
  glob_context_dirent(&context, GLOB_DIR, "test/");
  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );
  ASSERTEQ( GLOB_READDIR, glob_context_match(&context) );
  glob_context_dirent(&context, GLOB_FILE, "a.out");
  glob_context_dirent(&context, GLOB_FILE, "a.c");
  glob_context_dirent(&context, GLOB_FILE, "a.h");
  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );
  ASSERTEQ( GLOB_MATCH, glob_context_match(&context) );
  ASSERTEQ( GLOB_NOMATCH, glob_context_match(&context) );
  
  glob_context_destroy(&context);
  fnmatch_pattern_destroy(&pattern);
}