/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_h.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hadrider <hadrider@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 13:20:00 by hadrider          #+#    #+#             */
/*   Updated: 2026/09/09 13:20:00 by hadrider         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_before(t_request *a, t_request *b, t_scheduler policy)
{
	if (policy == EDF && a->deadline != b->deadline)
		return (a->deadline < b->deadline);
	if (policy == EDF && a->id != b->id)
		return (a->id > b->id);
	if (a->order != b->order)
		return (a->order < b->order);
	return (a->id < b->id);
}

void	heap_swap(t_request *a, t_request *b)
{
	t_request	t;

	t = *a;
	*a = *b;
	*b = t;
}

int	heap_resize(t_heap *h)
{
	t_request	*new_items;
	int			i;

	if (h->capacity == 0)
		h->capacity = 8;
	else
		h->capacity *= 2;
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
	return (1);
}

void	heap_sift_down(t_heap *h, t_scheduler policy)
{
	int	i;
	int	left;
	int	right;
	int	best;

	i = 0;
	while (1)
	{
		left = i * 2 + 1;
		if (left >= h->size)
			break ;
		right = left + 1;
		best = left;
		if (right < h->size
			&& heap_before(&h->items[right], &h->items[left], policy))
			best = right;
		if (!heap_before(&h->items[best], &h->items[i], policy))
			break ;
		heap_swap(&h->items[i], &h->items[best]);
		i = best;
	}
}
