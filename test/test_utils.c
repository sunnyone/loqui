#include "test_helper.h"

static int
test_utils_format(void)
{
   return FALSE;
}

int
main()
{
	int all=0, failed=0;

	DO_TEST(all, failed, test_utils_format);

	SHOW_RESULT_AND_EXIT(all, failed, utils);
}

