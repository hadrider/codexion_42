#include "codexion.h"

void	request_init(t_request *r, int id, unsigned long seq, long deadline)
{
	r->coder_id = id;
	r->arrival = seq;
	r->deadline = deadline;
}

static void	remove_request(t_heap *h, int cid, t_scheduler scheduler)
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
		r = h->items[i];
		if (r.coder_id != cid)
			heap_push(&tmp, r, scheduler);
		i++;
	}
	heap_destroy(h);
	*h = tmp;
}

static int	count_free_dongles(t_sim *s)
{
	int	i;
	int	free_count;

	i = 0;
	free_count = 0;
	while (i < s->count)
	{
		pthread_mutex_lock(&s->dongles[i].mutex);
		if (s->dongles[i].owner == -1
			&& s->dongles[i].reserved == -1
			&& now_ms() >= s->dongles[i].available_at)
			free_count++;
		pthread_mutex_unlock(&s->dongles[i].mutex);
		i++;
	}
	return (free_count);
}

static int	can_take(t_sim *s, int did, int cid)
{
	t_dongle	*d;

	d = &s->dongles[did];
	if (d->owner != -1 && d->owner != cid)
		return (0);
	if (d->reserved != -1 && d->reserved != cid)
		return (0);
	if (d->heap.size == 0)
		return (0);
	if (d->heap.items[0].coder_id != cid)
		return (0);
	if (now_ms() < d->available_at)
		return (0);
	return (1);
}

static int	take_dongle(t_sim *s, int did, int cid)
{
	t_dongle	*d;
	t_request	r;

	d = &s->dongles[did];
	if (!can_take(s, did, cid))
		return (0);
	heap_pop(&d->heap, &r, s->scheduler);
	d->owner = cid;
	d->reserved = cid;
	return (1);
}

static void	put_back_request(t_sim *s, int did, int cid)
{
	pthread_mutex_lock(&s->dongles[did].mutex);
	remove_request(&s->dongles[did].heap, cid, s->scheduler);
	pthread_mutex_unlock(&s->dongles[did].mutex);
}

int	dongles_acquire(t_sim *s, int first, int second,
	int cid, long deadline)
{
	t_request		r;
	unsigned long	seq;
	int				first_taken;
	int				second_taken;
	int				free_count;

	first_taken = 0;
	second_taken = 0;

	pthread_mutex_lock(&s->seq_mutex);
	s->arrival_seq++;
	seq = s->arrival_seq;
	pthread_mutex_unlock(&s->seq_mutex);

	request_init(&r, cid, seq, deadline);

	pthread_mutex_lock(&s->dongles[first].mutex);
	if (!heap_push(&s->dongles[first].heap, r, s->scheduler))
	{
		pthread_mutex_unlock(&s->dongles[first].mutex);
		return (0);
	}
	pthread_mutex_unlock(&s->dongles[first].mutex);

	pthread_mutex_lock(&s->dongles[second].mutex);
	if (!heap_push(&s->dongles[second].heap, r, s->scheduler))
	{
		pthread_mutex_unlock(&s->dongles[second].mutex);
		put_back_request(s, first, cid);
		return (0);
	}
	pthread_mutex_unlock(&s->dongles[second].mutex);

	while (!sim_stopped(s))
	{
		free_count = count_free_dongles(s);

		/*
		 * First try to get both dongles atomically.
		 */
		pthread_mutex_lock(&s->dongles[first].mutex);
		pthread_mutex_lock(&s->dongles[second].mutex);
		if (can_take(s, first, cid) && can_take(s, second, cid))
		{
			heap_pop(&s->dongles[first].heap, &r, s->scheduler);
			heap_pop(&s->dongles[second].heap, &r, s->scheduler);
			s->dongles[first].owner = cid;
			s->dongles[first].reserved = cid;
			s->dongles[second].owner = cid;
			s->dongles[second].reserved = cid;
			pthread_mutex_unlock(&s->dongles[second].mutex);
			pthread_mutex_unlock(&s->dongles[first].mutex);
			log_action(s, cid, "has taken a dongle");
			log_action(s, cid, "has taken a dongle");
			return (1);
		}
		pthread_mutex_unlock(&s->dongles[second].mutex);
		pthread_mutex_unlock(&s->dongles[first].mutex);

		/*
		 * If exactly one dongle is free, allow this coder
		 * to reserve it. This prevents multiple coders from
		 * holding one dongle each and deadlocking.
		 */
		if (free_count == 1)
		{
			if (!first_taken)
			{
				pthread_mutex_lock(&s->dongles[first].mutex);
				if (can_take(s, first, cid))
					first_taken = take_dongle(s, first, cid);
				pthread_mutex_unlock(&s->dongles[first].mutex);
				if (first_taken)
					log_action(s, cid, "has taken a dongle");
			}
			if (!second_taken)
			{
				pthread_mutex_lock(&s->dongles[second].mutex);
				if (can_take(s, second, cid))
					second_taken = take_dongle(s, second, cid);
				pthread_mutex_unlock(&s->dongles[second].mutex);
				if (second_taken)
					log_action(s, cid, "has taken a dongle");
			}
		}

		if (first_taken && second_taken)
			return (1);

		if (now_ms() >= deadline)
			break ;
		usleep(1000);
	}

	if (first_taken)
	{
		pthread_mutex_lock(&s->dongles[first].mutex);
		if (s->dongles[first].owner == cid)
		{
			s->dongles[first].owner = -1;
			s->dongles[first].reserved = -1;
		}
		pthread_cond_broadcast(&s->dongles[first].cond);
		pthread_mutex_unlock(&s->dongles[first].mutex);
	}
	else
		put_back_request(s, first, cid);

	if (second_taken)
	{
		pthread_mutex_lock(&s->dongles[second].mutex);
		if (s->dongles[second].owner == cid)
		{
			s->dongles[second].owner = -1;
			s->dongles[second].reserved = -1;
		}
		pthread_cond_broadcast(&s->dongles[second].cond);
		pthread_mutex_unlock(&s->dongles[second].mutex);
	}
	else
		put_back_request(s, second, cid);

	return (0);
}

void	dongle_release(t_sim *s, int did, int cid)
{
	t_dongle	*d;

	d = &s->dongles[did];
	pthread_mutex_lock(&d->mutex);
	if (d->owner == cid)
	{
		d->owner = -1;
		d->reserved = -1;
		d->available_at = now_ms() + s->cooldown;
		pthread_cond_broadcast(&d->cond);
	}
	pthread_mutex_unlock(&d->mutex);
}