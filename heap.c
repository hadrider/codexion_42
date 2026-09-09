#include "codexion.h"

static int before(t_request *a, t_request *b, t_scheduler policy)
{
	if (policy == EDF && a->deadline != b->deadline)
		return (a->deadline < b->deadline);
	if (policy == EDF && a->id != b->id)
		return (a->id > b->id);
	if (a->order != b->order)
		return (a->order < b->order);
	return (a->id < b->id);
}

static void swap(t_request *a, t_request *b)
{
	t_request t;

	t = *a;
	*a = *b;
	*b = t;
}

int heap_push(t_heap *h, t_request r, t_scheduler policy)
{
	t_request *new_items;
	int i;
	int parent;

	if (h->size == h->capacity)
	{
		h->capacity = h->capacity ? h->capacity * 2 : 8;
		new_items = malloc(sizeof(*new_items) * h->capacity);
		if (!new_items)
			return (0);
		i = 0;
		while (i < h->size)
		{
			new_items[i] = h->items[i];
			i++;
		}
		free(h->items);
		h->items = new_items;
	}
	i = h->size++;
	h->items[i] = r;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!before(&h->items[i], &h->items[parent], policy))
			break ;
		swap(&h->items[i], &h->items[parent]);
		i = parent;
	}
	return (1);
}

int heap_pop(t_heap *h, t_request *r, t_scheduler policy)
{
	int i;
	int left;
	int right;
	int best;

	if (!h->size)
		return (0);
	*r = h->items[0];
	--h->size;
	if (!h->size)
		return (1);
	h->items[0] = h->items[h->size];
	i = 0;
	while (1)
	{
		left = i * 2 + 1;
		if (left >= h->size)
			break ;
		right = left + 1;
		best = left;
		if (right < h->size && before(&h->items[right], &h->items[left], policy))
			best = right;
		if (!before(&h->items[best], &h->items[i], policy))
			break ;
		swap(&h->items[i], &h->items[best]);
		i = best;
	}
	return (1);
}

void heap_destroy(t_heap *h)
{
	free(h->items);
	h->items = NULL;
	h->size = 0;
	h->capacity = 0;
}
