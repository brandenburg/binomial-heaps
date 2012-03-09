/* heap.h -- Binomial Heaps
 *
 * Copyright (c) 2008, Bjoern B. Brandenburg <bbb [at] cs.unc.edu>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of North Carolina nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY  COPYRIGHT OWNER AND CONTRIBUTERS ``AS IS'' AND
 * ANY  EXPRESS OR  IMPLIED  WARRANTIES,  INCLUDING, BUT  NOT  LIMITED TO,  THE
 * IMPLIED WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO  EVENT SHALL THE  COPYRIGHT OWNER OR  CONTRIBUTERS BE
 * LIABLE  FOR  ANY  DIRECT,   INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
 * CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED  TO,  PROCUREMENT  OF
 * SUBSTITUTE GOODS  OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
 * INTERRUPTION)  HOWEVER CAUSED  AND ON  ANY THEORY  OF LIABILITY,  WHETHER IN
 * CONTRACT,  STRICT LIABILITY,  OR  TORT (INCLUDING  NEGLIGENCE OR  OTHERWISE)
 * ARISING IN ANY WAY  OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HEAP_H
#define HEAP_H

#define NOT_IN_HEAP UINT_MAX

struct heap_node {
	struct heap_node* 	parent;
	struct heap_node* 	next;
	struct heap_node* 	child;

	unsigned int 		degree;
	void*			value;
	struct heap_node**	ref;
};

struct heap {
	struct heap_node* 	head;
	/* We cache the minimum of the heap.
	 * This speeds up repeated peek operations.
	 */
	struct heap_node*	min;
};

/* item comparison function:
 * return 1 if a has higher prio than b, 0 otherwise
 */
typedef int (*heap_prio_t)(struct heap_node* a, struct heap_node* b);

static inline void heap_init(struct heap* heap)
{
	heap->head = NULL;
	heap->min  = NULL;
}

static inline void heap_node_init_ref(struct heap_node** _h, void* value)
{
	struct heap_node* h = *_h;
	h->parent = NULL;
	h->next   = NULL;
	h->child  = NULL;
	h->degree = NOT_IN_HEAP;
	h->value  = value;
	h->ref    = _h;
}

static inline void heap_node_init(struct heap_node* h, void* value)
{
	h->parent = NULL;
	h->next   = NULL;
	h->child  = NULL;
	h->degree = NOT_IN_HEAP;
	h->value  = value;
	h->ref    = NULL;
}

static inline void* heap_node_value(struct heap_node* h)
{
	return h->value;
}

static inline int heap_node_in_heap(struct heap_node* h)
{
	return h->degree != NOT_IN_HEAP;
}

static inline int heap_empty(struct heap* heap)
{
	return heap->head == NULL && heap->min == NULL;
}

/* make child a subtree of root */
static inline void __heap_link(struct heap_node* root,
			       struct heap_node* child)
{
	child->parent = root;
	child->next   = root->child;
	root->child   = child;
	root->degree++;
}

/* merge root lists */
static inline struct heap_node* __heap_merge(struct heap_node* a,
					     struct heap_node* b)
{
	struct heap_node* head = NULL;
	struct heap_node** pos = &head;

	while (a && b) {
		if (a->degree < b->degree) {
			*pos = a;
			a = a->next;
		} else {
			*pos = b;
			b = b->next;
		}
		pos = &(*pos)->next;
	}
	if (a)
		*pos = a;
	else
		*pos = b;
	return head;
}

/* reverse a linked list of nodes. also clears parent pointer */
static inline struct heap_node* __heap_reverse(struct heap_node* h)
{
	struct heap_node* tail = NULL;
	struct heap_node* next;

	if (!h)
		return h;

	h->parent = NULL;
	while (h->next) {
		next    = h->next;
		h->next = tail;
		tail    = h;
		h       = next;
		h->parent = NULL;
	}
	h->next = tail;
	return h;
}

static inline void __heap_min(heap_prio_t higher_prio, struct heap* heap,
			      struct heap_node** prev, struct heap_node** node)
{
	struct heap_node *_prev, *cur;
	*prev = NULL;

	if (!heap->head) {
		*node = NULL;
		return;
	}

	*node = heap->head;
	_prev = heap->head;
	cur   = heap->head->next;
	while (cur) {
		if (higher_prio(cur, *node)) {
			*node = cur;
			*prev = _prev;
		}
		_prev = cur;
		cur   = cur->next;
	}
}

static inline void __heap_union(heap_prio_t higher_prio, struct heap* heap,
				struct heap_node* h2)
{
	struct heap_node* h1;
	struct heap_node *prev, *x, *next;
	if (!h2)
		return;
	h1 = heap->head;
	if (!h1) {
		heap->head = h2;
		return;
	}
	h1 = __heap_merge(h1, h2);
	prev = NULL;
	x    = h1;
	next = x->next;
	while (next) {
		if (x->degree != next->degree ||
		    (next->next && next->next->degree == x->degree)) {
			/* nothing to do, advance */
			prev = x;
			x    = next;
		} else if (higher_prio(x, next)) {
			/* x becomes the root of next */
			x->next = next->next;
			__heap_link(x, next);
		} else {
			/* next becomes the root of x */
			if (prev)
				prev->next = next;
			else
				h1 = next;
			__heap_link(next, x);
			x = next;
		}
		next = x->next;
	}
	heap->head = h1;
}

static inline struct heap_node* __heap_extract_min(heap_prio_t higher_prio,
						   struct heap* heap)
{
	struct heap_node *prev, *node;
	__heap_min(higher_prio, heap, &prev, &node);
	if (!node)
		return NULL;
	if (prev)
		prev->next = node->next;
	else
		heap->head = node->next;
	__heap_union(higher_prio, heap, __heap_reverse(node->child));
	return node;
}

/* insert (and reinitialize) a node into the heap */
static inline void heap_insert(heap_prio_t higher_prio, struct heap* heap,
			       struct heap_node* node)
{
	struct heap_node *min;
	node->child  = NULL;
	node->parent = NULL;
	node->next   = NULL;
	node->degree = 0;
	if (heap->min && higher_prio(node, heap->min)) {
		/* swap min cache */
		min = heap->min;
		min->child  = NULL;
		min->parent = NULL;
		min->next   = NULL;
		min->degree = 0;
		__heap_union(higher_prio, heap, min);
		heap->min   = node;
	} else
		__heap_union(higher_prio, heap, node);
}

static inline void __uncache_min(heap_prio_t higher_prio, struct heap* heap)
{
	struct heap_node* min;
	if (heap->min) {
		min = heap->min;
		heap->min = NULL;
		heap_insert(higher_prio, heap, min);
	}
}

/* merge addition into target */
static inline void heap_union(heap_prio_t higher_prio,
			      struct heap* target, struct heap* addition)
{
	/* first insert any cached minima, if necessary */
	__uncache_min(higher_prio, target);
	__uncache_min(higher_prio, addition);
	__heap_union(higher_prio, target, addition->head);
	/* this is a destructive merge */
	addition->head = NULL;
}

static inline struct heap_node* heap_peek(heap_prio_t higher_prio,
					  struct heap* heap)
{
	if (!heap->min)
		heap->min = __heap_extract_min(higher_prio, heap);
	return heap->min;
}

static inline struct heap_node* heap_take(heap_prio_t higher_prio,
					  struct heap* heap)
{
	struct heap_node *node;
	if (!heap->min)
		heap->min = __heap_extract_min(higher_prio, heap);
	node = heap->min;
	heap->min = NULL;
	if (node)
		node->degree = NOT_IN_HEAP;
	return node;
}

static inline void heap_decrease(heap_prio_t higher_prio, struct heap* heap,
				 struct heap_node* node)
{
	struct heap_node *parent;
	struct heap_node** tmp_ref;
	void* tmp;

	/* node's priority was decreased, we need to update its position */
	if (!node->ref)
		return;
	if (heap->min != node) {
		if (heap->min && higher_prio(node, heap->min))
			__uncache_min(higher_prio, heap);
		/* bubble up */
		parent = node->parent;
		while (parent && higher_prio(node, parent)) {
			/* swap parent and node */
			tmp           = parent->value;
			parent->value = node->value;
			node->value   = tmp;
			/* swap references */
			if (parent->ref)
				*(parent->ref) = node;
			*(node->ref)   = parent;
			tmp_ref        = parent->ref;
			parent->ref    = node->ref;
			node->ref      = tmp_ref;
			/* step up */
			node   = parent;
			parent = node->parent;
		}
	}
}

static inline void heap_delete(heap_prio_t higher_prio, struct heap* heap,
			       struct heap_node* node)
{
	struct heap_node *parent, *prev, *pos;
	struct heap_node** tmp_ref;
	void* tmp;

	if (!node->ref) /* can only delete if we have a reference */
		return;
	if (heap->min != node) {
		/* bubble up */
		parent = node->parent;
		while (parent) {
			/* swap parent and node */
			tmp           = parent->value;
			parent->value = node->value;
			node->value   = tmp;
			/* swap references */
			if (parent->ref)
				*(parent->ref) = node;
			*(node->ref)   = parent;
			tmp_ref        = parent->ref;
			parent->ref    = node->ref;
			node->ref      = tmp_ref;
			/* step up */
			node   = parent;
			parent = node->parent;
		}
		/* now delete:
		 * first find prev */
		prev = NULL;
		pos  = heap->head;
		while (pos != node) {
			prev = pos;
			pos  = pos->next;
		}
		/* we have prev, now remove node */
		if (prev)
			prev->next = node->next;
		else
			heap->head = node->next;
		__heap_union(higher_prio, heap, __heap_reverse(node->child));
	} else
		heap->min = NULL;
	node->degree = NOT_IN_HEAP;
}

#endif /* HEAP_H */
