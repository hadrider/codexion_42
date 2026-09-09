/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hadrider <hadrider@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 13:05:30 by hadrider          #+#    #+#             */
/*   Updated: 2026/09/09 13:59:39 by hadrider         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_push(t_heap *h, t_request r, t_scheduler policy)
{
	int	i;
	int	parent;

	if (h->size == h->capacity && !heap_resize(h))
		return (0);
	i = h->size++;
	h->items[i] = r;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!heap_before(&h->items[i], &h->items[parent], policy))
			break ;
		heap_swap(&h->items[i], &h->items[parent]);
		i = parent;
	}
	return (1);
}

int	heap_pop(t_heap *h, t_request *r, t_scheduler policy)
{
	if (!h->size)
		return (0);
	*r = h->items[0];
	--h->size;
	if (!h->size)
		return (1);
	h->items[0] = h->items[h->size];
	heap_sift_down(h, policy);
	return (1);
}

void	heap_destroy(t_heap *h)
{
	free(h->items);
	h->items = NULL;
	h->size = 0;
	h->capacity = 0;
}

void	heap_remove(t_heap *h, int id, t_scheduler policy)
{
	t_heap		tmp;
	t_request	r;
	int			i;

	tmp.items = NULL;
	tmp.size = 0;
	tmp.capacity = 0;
	i = 0;
	while (i < h->size)
	{
		r = h->items[i++];
		if (r.id != id)
			heap_push(&tmp, r, policy);
	}
	heap_destroy(h);
	*h = tmp;
}
