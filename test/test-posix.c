#include "fnmatch.h"
#include "test.h"

typedef struct test_glob_s {
  const char* pattern;
  int flags;
} test_glob_t;

static const test_glob_t _data[] = {

  { "test/test-*.c", 0 }

};

TEST( test_glob, _data, const test_glob_t* data ) {
  WARN( "Not implemented yet.\n" );
}
