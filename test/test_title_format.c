#include "test_helper.h"
#include "loqui_title_format.h"
#include <glib.h>

static int
test_title_format(void)
{
   LoquiTitleFormat *ltf;
   gchar *result;

   ltf = loqui_title_format_new();
   loqui_title_format_register_variable(ltf, "channel", "#hoge");
   loqui_title_format_register_variable(ltf, "nick", "fuga");
   loqui_title_format_register_variable(ltf, "message", "message");
   
   result = loqui_title_format_parse(ltf, "<%channel%:%nick%> %message%", NULL);
   g_print("result: %s\n", result);
   g_return_val_if_fail(strcmp("<#hoge:fuga> message", result) == 0, FALSE);
   g_free(result);

   result = loqui_title_format_parse(ltf, "[%undefined_variable%]<%channel%>", NULL);
   g_print("result2: %s\n", result);
   g_return_val_if_fail(result != NULL, FALSE);
   g_return_val_if_fail(strcmp("<#hoge>", result) == 0, FALSE);
   g_free(result);

   loqui_title_format_free(ltf); 

   return TRUE;
}

int
main()
{
	int all=0, failed=0;

	DO_TEST(all, failed, test_title_format);

	SHOW_RESULT_AND_EXIT(all, failed, utils);
}

