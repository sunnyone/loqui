#include "test_helper.h"
#include "loqui_string_tokenizer.h"

static int
test_string_tokenizer(void)
{
	LoquiStringTokenizer *st;

	st = loqui_string_tokenizer_new("hoge:fuga hige", "\t:");
	gchar d;
	g_print("%d\n", loqui_string_tokenizer_count_tokens(st));
	g_print("%s\n", loqui_string_tokenizer_peek_next_token(st, &d));
	g_print("del: %c\n", d);
	g_print("%s\n", loqui_string_tokenizer_next_token(st, NULL));
	loqui_string_tokenizer_skip_char(st);
	g_print("cur: %c\n", loqui_string_tokenizer_peek_cur_char(st));
	g_print("%s\n", loqui_string_tokenizer_has_more_tokens(st) ? "TRUE" : "FALSE");
	g_print("%s\n", loqui_string_tokenizer_next_token(st, NULL));
	g_print("%s\n", loqui_string_tokenizer_has_more_tokens(st) ? "TRUE" : "FALSE");
	g_print("%s\n", loqui_string_tokenizer_next_token(st, NULL));
	loqui_string_tokenizer_free(st);
	return TRUE;
}

static int
test_string_tokenizer_not_split(void)
{
	LoquiStringTokenizer *st;
	const gchar *s;

	st = loqui_string_tokenizer_new("testtest", " ");
	s = loqui_string_tokenizer_next_token(st, NULL);

	if (strcmp(s, "testtest") != 0) {
		return FALSE;
	}
	if (loqui_string_tokenizer_next_token(st, NULL) != NULL)
		return FALSE;

	return TRUE;
}

int
main()
{
	int all=0, failed=0;

	DO_TEST(all, failed, test_string_tokenizer);
	DO_TEST(all, failed, test_string_tokenizer_not_split);

	SHOW_RESULT_AND_EXIT(all, failed, utils);
}

