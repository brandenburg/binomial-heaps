#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "heap.h"

struct token {
	int prio;
	const char* str;
};

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

struct token tokens1[] = {
	{24, "all"},  {16, "star"},  {9, "true.\nSinging"},  {7, "clear"},
	{25, "praises"},  {13, "to"},  {5, "Heel"},  {6, "voices\nRinging"},
	{26, "thine."},  {21, "shine\nCarolina"},  {117, "Rah,"},  {102, "Tar"},
	{108, "bred\nAnd"},  {125, "Rah!"},  {107, "Heel"},  {118, "Rah,"},
	{111, "die\nI'm"},  {115, "dead.\nSo"},  {120, "Rah,"},
	{121, "Car'lina-lina\nRah,"},  {109, "when"},  {105, "a"},
	{123, "Car'lina-lina\nRah!"},  {110, "I"},  {114, "Heel"},  {101, "a"},
	{106, "Tar"},  {18, "all\nClear"},  {14, "the"}
};

struct token tokens2[] = {
	{113, "Tar"},  {124, "Rah!"},  {112, "a"},  {103, "Heel"},
	{104, "born\nI'm"},  {122, "Rah,"},  {119, "Car'lina-lina\nRah,"},
	{2, "sound"},  {20, "radiance"},  {12, "N-C-U.\nHail"},
	{10, "Carolina's"},  {3, "of"},  {17, "of"},  {23, "gem.\nReceive"},
	{19, "its"},  {0, "\nHark"},  {22, "priceless"},  {4, "Tar"},
	{1, "the"},  {8, "and"},  {15, "brightest"},
	{11, "praises.\nShouting"},  {100, "\nI'm"},  {116, "it's"}
};

#define line "\n==================================="

struct token layout[] = {
	{90, line}, {-2, line}, {200, line}, {201, "\n\n"}
};


struct token title[] = {
	{1000, "\nUNC Alma Mater:"}, {120, "\nUNC Fight Song:"}
};

struct token bad[] = {
	{666, "Dook"}, {666666, "Blue Devils"}
};

static int token_cmp(struct heap_node* _a, struct heap_node* _b)
{
	struct token *a, *b;
	a = (struct token*) heap_node_value(_a);
	b = (struct token*) heap_node_value(_b);
	return a->prio < b->prio;
}

static void add_token(struct heap* heap, struct token* tok)
{
	struct heap_node* hn = malloc(sizeof(struct heap_node));
	heap_node_init(hn, tok);
	heap_insert(token_cmp, heap, hn);
}

static void add_token_ref(struct heap* heap, struct token* tok,
			  struct heap_node** hn)
{
	*hn = malloc(sizeof(struct heap_node));
	heap_node_init_ref(hn, tok);
	heap_insert(token_cmp, heap, *hn);
}

static void add_tokens(struct heap* heap, struct token* tok, int len)
{
	int i;
	for (i = 0; i < len; i++)
		add_token(heap, tok + i);
}

int main(int argc __attribute__((unused)), char**  argv __attribute__((unused)))
{
	struct heap h1, h2, h3;
	struct heap_node* hn;
	struct heap_node *t1, *t2, *b1, *b2;
	struct token *tok;

	heap_init(&h1);
	heap_init(&h2);
	heap_init(&h3);

	add_tokens(&h1, tokens1, LENGTH(tokens1));
	add_tokens(&h2, tokens2, LENGTH(tokens2));
	add_tokens(&h3, layout, LENGTH(layout));

	add_token_ref(&h3, title, &t1);
	add_token_ref(&h2, title + 1, &t2);

	heap_union(token_cmp, &h2, &h3);
	heap_union(token_cmp, &h1, &h2);

	add_token_ref(&h3, bad, &b1);
	add_token_ref(&h3, bad + 1, &b2);

	heap_union(token_cmp, &h1, &h3);

	heap_delete(token_cmp, &h1, b1);
	free(b1);
	heap_delete(token_cmp, &h1, b2);
	free(b2);

	title[0].prio = -1;
	title[1].prio = 99;

	heap_decrease(token_cmp, &h1, t1);
	heap_decrease(token_cmp, &h1, t2);

	printf("htest:\n");
	while (!heap_empty(&h1)) {
		hn  = heap_take(token_cmp, &h1);
		tok = heap_node_value(hn);
		printf("%s ", tok->str);
		free(hn);
	}
	return 0;
}
