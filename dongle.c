/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hadrider <hadrider@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 14:00:38 by hadrider          #+#    #+#             */
/*   Updated: 2026/09/09 14:01:05 by hadrider         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	put_dongle(t_sim *s, int did, int id)
{
	t_dongle	*d;

	d = &s->dongles[did];
	pthread_mutex_lock(&d->mutex);
	if (d->owner == id)
	{
		d->owner = -1;
		d->available_at = now_ms() + s->cooldown;
	}
	pthread_mutex_unlock(&d->mutex);
	wake_all(s);
}

static int	dongles_ready(t_sim *s, t_coder *c)
{
	t_dongle	*a;
	t_dongle	*b;
	long		now;

	a = &s->dongles[c->left];
	b = &s->dongles[c->right];
	now = now_ms();
	return (a->owner == -1 && b->owner == -1
		&& now >= a->available_at && now >= b->available_at);
}

static void	lock_pair(t_sim *s, t_coder *c)
{
	t_dongle	*a;
	t_dongle	*b;

	a = &s->dongles[c->left];
	b = &s->dongles[c->right];
	if (c->left < c->right)
	{
		pthread_mutex_lock(&a->mutex);
		pthread_mutex_lock(&b->mutex);
	}
	else
	{
		pthread_mutex_lock(&b->mutex);
		pthread_mutex_lock(&a->mutex);
	}
}

static void	unlock_pair(t_sim *s, t_coder *c)
{
	pthread_mutex_unlock(&s->dongles[c->left].mutex);
	pthread_mutex_unlock(&s->dongles[c->right].mutex);
}

static int	take_pair(t_sim *s, t_coder *c)
{
	t_request	r;
	int			ok;

	ok = 0;
	lock_pair(s, c);
	if (s->queue.size && s->queue.items[0].id == c->id
		&& dongles_ready(s, c))
	{
		heap_pop(&s->queue, &r, s->scheduler);
		s->dongles[c->left].owner = c->id;
		s->dongles[c->right].owner = c->id;
		ok = 1;
	}
	unlock_pair(s, c);
	return (ok);
}

static int	queue_request(t_sim *s, int id, long deadline)
{
	t_request	r;

	r.id = id;
	pthread_mutex_lock(&s->queue_mutex);
	r.order = ++s->order;
	r.deadline = deadline;
	if (!heap_push(&s->queue, r, s->scheduler))
	{
		pthread_mutex_unlock(&s->queue_mutex);
		return (0);
	}
	return (1);
}

static void	wait_for_pair(t_sim *s, t_coder *c)
{
	while (!is_stopped(s))
	{
		if (take_pair(s, c))
			break ;
		pthread_mutex_unlock(&s->queue_mutex);
		usleep(1000);
		pthread_mutex_lock(&s->queue_mutex);
	}
}

int	take_dongles(t_sim *s, t_coder *c, long deadline)
{
	if (c->left == c->right)
		return (0);
	if (!queue_request(s, c->id, deadline))
		return (0);
	wait_for_pair(s, c);
	if (is_stopped(s))
		heap_remove(&s->queue, c->id, s->scheduler);
	pthread_mutex_unlock(&s->queue_mutex);
	if (is_stopped(s))
		return (0);
	log_action(s, c->id, "has taken a dongle");
	log_action(s, c->id, "has taken a dongle");
	return (1);
}
