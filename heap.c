#include "codexion.h"

static int	before(t_request *a, t_request *b, t_scheduler s)
{
	if (s == SCHED_POLICY_EDF && a->deadline != b->deadline)
		return (a->deadline < b->deadline);
	if (a->arrival != b->arrival)
		return (a->arrival < b->arrival);
	return (a->coder_id < b->coder_id);
}

static void	swap(t_request *a, t_request *b)
{
	t_request	t;

	t = *a;
	*a = *b;
	*b = t;
}

int	heap_push(t_heap *h, t_request r, t_scheduler s)
{
	int	i;
	int	p;
	t_request	*n;

	if (h->size == h->capacity)
	{
		h->capacity = (h->capacity == 0) ? 4 : h->capacity * 2;
		n = malloc(sizeof(*n) * h->capacity);
		if (!n)
			return (0);
		if (h->items)
		{
			int k;

			k = 0;
			while (k < h->size)
			{
				n[k] = h->items[k];
				k++;
			}
			free(h->items);
		}
		h->items = n;
	}
	i = h->size++;
	h->items[i] = r;
	while (i > 0)
	{
		p = (i - 1) / 2;
		if (!before(&h->items[i], &h->items[p], s))
			break ;
		swap(&h->items[i], &h->items[p]);
		i = p;
	}
	return (1);
}

int	heap_pop(t_heap *h, t_request *r, t_scheduler s)
{
	int	i;
	int	l;
	int	right;
	int	best;
	t_request tmp;

	if (h->size == 0)
		return (0);
	*r = h->items[0];
	h->size--;
	if (h->size == 0)
		return (1);
	h->items[0] = h->items[h->size];
	i = 0;
	while (1)
	{
		l = i * 2 + 1;
		right = l + 1;
		if (l >= h->size)
			break ;
		best = l;
		if (right < h->size && before(&h->items[right], &h->items[l],
				s))
			best = right;
		if (!before(&h->items[best], &h->items[i], s))
			break ;
		tmp = h->items[i];
		h->items[i] = h->items[best];
		h->items[best] = tmp;
		i = best;
	}
	return (1);
}

void	heap_destroy(t_heap *h)
{
	free(h->items);
	h->items = NULL;
	h->size = 0;
	h->capacity = 0;
}
