#include <stdlib.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define DO_TEST(func) { \
  printf("Testing %s...\n", __PRETTY_FUNCTION__); \
  if (func()) { \
    printf("Test Passed %s.\n", __PRETTY_FUNCTION__); \
  } else { \
    printf("TEST FAILED (%s).\n" __PRETTY_FUNCTION__); \
    exit(1); \
  } \
} 

