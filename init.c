/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hadrider <hadrider@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 13:59:54 by hadrider          #+#    #+#             */
/*   Updated: 2026/09/09 13:59:55 by hadrider         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_mutexes(t_sim *s)
{
	if (pthread_mutex_init(&s->state_mutex, NULL))
		return (0);
	s->state_ready = 1;
	if (pthread_mutex_init(&s->log_mutex, NULL))
		return (0);
	s->log_ready = 1;
	if (pthread_mutex_init(&s->queue_mutex, NULL))
		return (0);
	s->queue_ready = 1;
	if (pthread_cond_init(&s->queue_cond, NULL))
		return (0);
	return (1);
}

static int	init_dongles(t_sim *s)
{
	int	i;

	s->dongles = malloc(sizeof(*s->dongles) * s->count);
	if (!s->dongles)
		return (0);
	i = 0;
	while (i < s->count)
	{
		memset(&s->dongles[i], 0, sizeof(*s->dongles));
		if (pthread_mutex_init(&s->dongles[i].mutex, NULL))
			return (0);
		s->dongles[i].owner = -1;
		s->dongles[i].available_at = s->start;
		s->initialized_dongles++;
		i++;
	}
	return (1);
}

static void	init_coder(t_sim *s, int i)
{
	s->coders[i].id = i + 1;
	s->coders[i].left = i;
	s->coders[i].right = (i + 1) % s->count;
	s->coders[i].last_compile = s->start;
	s->coders[i].sim = s;
}

int	init_sim(t_sim *s)
{
	int	i;

	s->start = now_ms();
	if (!init_mutexes(s) || !init_dongles(s))
		return (0);
	s->coders = malloc(sizeof(*s->coders) * s->count);
	if (!s->coders)
		return (0);
	i = 0;
	while (i < s->count)
		init_coder(s, i++);
	return (1);
}

void	destroy_sim(t_sim *s)
{
	int	i;

	heap_destroy(&s->queue);
	if (s->dongles)
	{
		i = 0;
		while (i < s->initialized_dongles)
			pthread_mutex_destroy(&s->dongles[i++].mutex);
		free(s->dongles);
	}
	free(s->coders);
	if (s->queue_ready)
	{
		pthread_cond_destroy(&s->queue_cond);
		pthread_mutex_destroy(&s->queue_mutex);
	}
	if (s->log_ready)
		pthread_mutex_destroy(&s->log_mutex);
	if (s->state_ready)
		pthread_mutex_destroy(&s->state_mutex);
}
