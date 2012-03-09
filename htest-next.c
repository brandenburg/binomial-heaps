#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

#include "heap.h"

#define separator "\n------------------------------\n"

struct task {
	unsigned long long dline;
	int pid;
};

static inline int __dl_time_before(unsigned long long a, unsigned long long b)
{
	return (long long)(a - b) < 0;
}

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

struct task tasks1[] = {
	{24, 1},  {16, 2},  {9, 3},  {7, 4},
	{25, 5},  {13, 6},  {6, 8},
	{26, 9},  {21, 10},  {117, 11},  {102, 12},
	{108, 13},  {125, 14},  {107, 15},  {118, 16}
};

struct task tasks2[] = {
	{12, 17}, {4, 18}, {4, 19}, {5, 7}
};

static int task_cmp(struct heap_node* _a, struct heap_node* _b)
{
	struct task *a, *b;
	a = (struct task*) heap_node_value(_a);
	b = (struct task*) heap_node_value(_b);
	return __dl_time_before(a->dline, b->dline);
}

static void add_task(struct heap* heap, struct task* tsk)
{
	struct heap_node* hn = malloc(sizeof(struct heap_node));
	heap_node_init(hn, tsk);
	heap_insert(task_cmp, heap, hn);
}

static void add_task_ref(struct heap* heap, struct task* tsk,
			  struct heap_node** hn)
{
	*hn = malloc(sizeof(struct heap_node));
	heap_node_init_ref(hn, tsk);
	heap_insert(task_cmp, heap, *hn);
}

static void add_tasks(struct heap* heap, struct task* tsk, int len)
{
	int i;
	for (i = 0; i < len; i++)
		add_task(heap, tsk + i);
}

int main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{
	struct heap h1;
	struct heap_node* hn;
	struct heap_node *b1, *b2, *b3;
	struct task *tsk;
	int i;

	heap_init(&h1);

	add_tasks(&h1, tasks1, LENGTH(tasks1));

	printf("htest: peek task repeatedly\n");
	for (i = 0; i < 3; i++) {
		hn  = heap_peek(task_cmp, &h1);
		tsk = heap_node_value(hn);
		printf("(pid:%d, dline:%llu) ", tsk->pid, (unsigned long long) tsk->dline);
	}
	printf(separator);

	printf("\nhtest: remove one task\n");
	hn  = heap_take(task_cmp, &h1);
	tsk = heap_node_value(hn);
	printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
	free(hn);
	printf(separator);

	printf("\nhtest: peek task\n");
	hn  = heap_peek(task_cmp, &h1);
	tsk = heap_node_value(hn);
	printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
	printf(separator);

	printf("\nhtest: peek task next\n");
	hn  = heap_peek_next(task_cmp, &h1);
	tsk = heap_node_value(hn);
	printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
	printf(separator);

	printf("\nhtest: empty heap\n");
	while (!heap_empty(&h1)) {
		hn  = heap_take(task_cmp, &h1);
		tsk = heap_node_value(hn);
		printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
		free(hn);
	}
	printf(separator);

	printf("\nhtest: insert a task, peek and peek next\n");
	add_task(&h1, tasks1);
	hn  = heap_peek(task_cmp, &h1);
	tsk = heap_node_value(hn);
	printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
	hn  = heap_peek_next(task_cmp, &h1);
	if (hn) {
		tsk = heap_node_value(hn);
		printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
	} else
		printf("no task next!\n");
	printf("\nhtest: then remove\n");
	hn  = heap_take(task_cmp, &h1);
	tsk = heap_node_value(hn);
	printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
	free(hn);
	printf(separator);

	printf("\nhtest: insert all tasks\n");
	add_tasks(&h1, tasks1, LENGTH(tasks1));
	printf("\nhtest: insert two tasks by reference\n");
	add_task_ref(&h1, tasks2, &b1);
	add_task_ref(&h1, tasks2 + 1, &b2);
	printf("(pid:%d, dline:%llu) (pid:%d, dline:%llu)",
			tasks2[0].pid, tasks2[0].dline, tasks2[1].pid,
			tasks2[1].dline);
	printf("\nhtest: peek task\n");
	hn  = heap_peek(task_cmp, &h1);
	tsk = heap_node_value(hn);
	printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
	printf("\nhtest: peek task next\n");
	hn  = heap_peek_next(task_cmp, &h1);
	tsk = heap_node_value(hn);
	printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
	printf("\nhtest: delete the first one\n");
	heap_delete(task_cmp, &h1, b1);
	free(b1);
	printf("\nhtest: decrease the second one prio\n");
	tasks2[1].dline = 3;
	printf("(pid:%d, dline:%llu)", tasks2[1].pid, tasks2[1].dline);
	heap_decrease(task_cmp, &h1, b2);
	printf("\nhtest: insert a new task by reference (will be "
			"the new next)\n");
	add_task_ref(&h1, tasks2 + 2, &b1);
	printf("\nhtest: and decrease its prio\n");
	tasks2[2].dline = 2;
	printf("(pid:%d, dline:%llu)", tasks2[2].pid, tasks2[2].dline);
	heap_decrease(task_cmp, &h1, b1);
	printf("\nhtest: peek task\n");
	hn  = heap_peek(task_cmp, &h1);
	tsk = heap_node_value(hn);
	printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
	printf("\nhtest: peek task next\n");
	hn  = heap_peek_next(task_cmp, &h1);
	tsk = heap_node_value(hn);
	printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
	printf("\nhtest: ok.. let's now insert another task by reference\n");
	add_task_ref(&h1, tasks2 + 3, &b3);
	printf("(pid:%d, dline:%llu)", tasks2[3].pid, tasks2[3].dline);
	printf("\nhtest: and decrease its prio\n");
	tasks2[3].dline = 1;
	printf("(pid:%d, dline:%llu)", tasks2[3].pid, tasks2[3].dline);
	heap_decrease(task_cmp, &h1, b3);
	printf(separator);

	printf("\nhtest: empty heap\n");
	while (!heap_empty(&h1)) {
		hn  = heap_take(task_cmp, &h1);
		tsk = heap_node_value(hn);
		printf("(pid:%d, dline:%llu) ", tsk->pid, tsk->dline);
		free(hn);
	}
	printf("\n");
	return 0;
}
