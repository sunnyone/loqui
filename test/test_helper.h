#include <gnet.h>
#include <gtk/gtk.h>
#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int debug_mode = 0;
int show_msg_mode = 0;
int send_status_commands_mode = 0;

#define DO_TEST(all, failed, func) { \
  fprintf(stderr, "Testing %s...\n",  # func); \
  all++; \
  if (func()) { \
    fprintf(stderr, " Test Passed %s.\n",  # func); \
  } else { \
    fprintf(stderr, " TEST FAILED (%s).\n", # func); \
    failed++; \
  } \
}

#define SHOW_RESULT_AND_EXIT(all, failed, name) { \
  fprintf(stderr, "Test " # name " result: %d/%d\n", all - failed, all); \
  exit(0); \
}
