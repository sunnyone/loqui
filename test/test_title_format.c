#include "test_helper.h"
#include "loqui_title_format.h"
#include <glib.h>

static int
test_title_format(void)
{
   LoquiTitleFormat *ltf;
   gchar *result;

   ltf = loqui_title_format_new();
   loqui_title_format_register_variables(ltf, "channel", "#hoge", "nick", "fuga", "message", "message", NULL);
   
   loqui_title_format_parse(ltf, "<%channel%:%nick%> %message%", NULL);
   result = loqui_title_format_fetch(ltf);
   g_print("result: %s\n", result);
   g_return_val_if_fail(strcmp("<#hoge:fuga> message", result) == 0, FALSE);
   g_free(result);

   loqui_title_format_parse(ltf, "[ %undefined_variable% ]<%channel%>", NULL);
   result = loqui_title_format_fetch(ltf);
   g_print("result2: %s\n", result);
   g_return_val_if_fail(result != NULL, FALSE);
   g_return_val_if_fail(strcmp("<#hoge>", result) == 0, FALSE);
   g_free(result);

   loqui_title_format_parse(ltf, "'[ %undefined_variable% ]'<%channel%>//hogehogehogehoge%channel%hoge\nfuga", NULL);
   result = loqui_title_format_fetch(ltf);
   g_print("result3: %s\n", result);
   g_return_val_if_fail(result != NULL, FALSE);
   g_return_val_if_fail(strcmp("[ %undefined_variable% ]<#hoge>fuga", result) == 0, FALSE);
   g_free(result);
   
   if (!loqui_title_format_parse(ltf, "aaa $if($if(0,,nested-false),true,false) bbb", NULL))
	g_warning("Failed to parse");
   result = loqui_title_format_fetch(ltf);
   g_print("result4: %s\n", result);
   g_return_val_if_fail(result != NULL, FALSE);
   g_return_val_if_fail(strcmp("aaa true bbb", result) == 0, FALSE);
   g_free(result);

   loqui_title_format_parse(ltf, "$num(4,3)", NULL);
   result = loqui_title_format_fetch(ltf);
   g_print("result5: %s\n", result);
   g_return_val_if_fail(result != NULL, FALSE);
   g_return_val_if_fail(strcmp("004", result) == 0, FALSE);
   g_free(result);

   loqui_title_format_parse(ltf, "$pad(hoge,9, )", NULL);
   result = loqui_title_format_fetch(ltf);
   g_print("result6: %s\n", result);
   g_return_val_if_fail(result != NULL, FALSE);
   g_return_val_if_fail(strcmp("hoge     ", result) == 0, FALSE);
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

