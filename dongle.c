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

static void	lock_pair(t_dongle *a, t_dongle *b)
{
	if (a < b)
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

static void	unlock_pair(t_dongle *a, t_dongle *b)
{
	pthread_mutex_unlock(&a->mutex);
	pthread_mutex_unlock(&b->mutex);
}

static int	pair_ready(t_sim *sim, int first, int second, int cid)
{
	t_dongle	*a;
	t_dongle	*b;
	long		now;

	a = &sim->dongles[first];
	b = &sim->dongles[second];
	now = now_ms();
	if (a->heap.size == 0 || b->heap.size == 0)
		return (0);
	if (a->heap.items[0].coder_id != cid
		|| b->heap.items[0].coder_id != cid)
		return (0);
	if (a->owner != -1 || b->owner != -1)
		return (0);
	if (now < a->available_at || now < b->available_at)
		return (0);
	return (1);
}

int	dongles_acquire(t_sim *sim, int first, int second,
	int cid, long deadline)
{
	t_request	r;
	unsigned long	seq;
	int			pushed_first;
	int			pushed_second;
	t_dongle	*a;
	t_dongle	*b;

	if (first == second)
		return (0);
	a = &sim->dongles[first];
	b = &sim->dongles[second];

	pthread_mutex_lock(&sim->seq_mutex);
	sim->arrival_seq++;
	seq = sim->arrival_seq;
	pthread_mutex_unlock(&sim->seq_mutex);

	request_init(&r, cid, seq, deadline);

	pthread_mutex_lock(&sim->resource_mutex);

	lock_pair(a, b);
	pushed_first = heap_push(&a->heap, r, sim->scheduler);
	pushed_second = 0;
	if (pushed_first)
		pushed_second = heap_push(&b->heap, r, sim->scheduler);
	if (!pushed_first || !pushed_second)
	{
		if (pushed_first)
			remove_request(&a->heap, cid, sim->scheduler);
		if (pushed_second)
			remove_request(&b->heap, cid, sim->scheduler);
		unlock_pair(a, b);
		pthread_mutex_unlock(&sim->resource_mutex);
		return (0);
	}
	unlock_pair(a, b);

	while (!sim_stopped(sim))
	{
		lock_pair(a, b);
		if (pair_ready(sim, first, second, cid))
		{
			heap_pop(&a->heap, &r, sim->scheduler);
			heap_pop(&b->heap, &r, sim->scheduler);
			a->owner = cid;
			b->owner = cid;
			unlock_pair(a, b);
			pthread_mutex_unlock(&sim->resource_mutex);
			log_action(sim, cid, "has taken a dongle");
			log_action(sim, cid, "has taken a dongle");
			return (1);
		}
		unlock_pair(a, b);
		pthread_cond_wait(&sim->resource_cond, &sim->resource_mutex);
	}

	lock_pair(a, b);
	remove_request(&a->heap, cid, sim->scheduler);
	remove_request(&b->heap, cid, sim->scheduler);
	unlock_pair(a, b);
	pthread_mutex_unlock(&sim->resource_mutex);
	return (0);
}

void	dongle_release(t_sim *sim, int did, int cid)
{
	t_dongle	*d;

	pthread_mutex_lock(&sim->resource_mutex);
	d = &sim->dongles[did];
	pthread_mutex_lock(&d->mutex);
	if (d->owner == cid)
	{
		d->owner = -1;
		d->available_at = now_ms() + sim->cooldown;
		pthread_cond_broadcast(&d->cond);
	}
	pthread_mutex_unlock(&d->mutex);
	pthread_cond_broadcast(&sim->resource_cond);
	pthread_mutex_unlock(&sim->resource_mutex);
}
