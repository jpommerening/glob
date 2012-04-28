#include "fnmatch.h"
#include "glob.h"
#include "test.h"

typedef struct test_glob_s {
  const char* pattern;
  int flags;
  int (*errfunc) (const char*, int);
  int result;
  const char* files[10];
  
} test_glob_t;

static const test_glob_t _data[] = {

  { "test/test-*.c", 0, NULL, GLOB_NOSYS,
    { NULL } }
    
};

TEST( test_glob, _data, const test_glob_t* data ) {
  glob_t g;
  int r = glob( data->pattern, data->flags, data->errfunc, &g );
  ASSERTEQ( r, data->result );
  globfree(&g);
}
