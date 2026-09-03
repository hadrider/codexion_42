#include "codexion.h"

void	request_init(t_request *r, int id, unsigned long seq, long deadline)
{
	r->coder_id = id;
	r->arrival = seq;
	r->deadline = deadline;
}

int	dongle_acquire(t_sim *sim, int did, int cid, long deadline)
{
	t_dongle	*d;
	t_request	r;
	struct timespec	ts;
	int	ok;

	d = &sim->dongles[did];
	pthread_mutex_lock(&sim->seq_mutex);
	sim->arrival_seq++;
	request_init(&r, cid, sim->arrival_seq, deadline);
	pthread_mutex_unlock(&sim->seq_mutex);
	pthread_mutex_lock(&d->mutex);
	if (!heap_push(&d->heap, r, sim->scheduler))
	{
		pthread_mutex_unlock(&d->mutex);
		return (0);
	}
	while (!sim_stopped(sim))
	{
		if (d->heap.size > 0 && d->heap.items[0].coder_id == cid
			&& d->owner == -1 && now_ms() >= d->available_at)
		{
			heap_pop(&d->heap, &r, sim->scheduler);
			d->owner = cid;
			pthread_mutex_unlock(&d->mutex);
			log_action(sim, cid, "has taken a dongle");
			return (1);
		}
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += 0;
		ts.tv_nsec += 2000000;
		if (ts.tv_nsec >= 1000000000L)
		{
			ts.tv_sec++;
			ts.tv_nsec -= 1000000000L;
		}
		pthread_cond_timedwait(&d->cond, &d->mutex, &ts);
	}
	ok = 0;
	/* Remove our pending request. Rebuild the heap without this request. */
	{
		t_heap	tmp;
		t_request	x;
		int	i;

		tmp.items = NULL;
		tmp.size = 0;
		tmp.capacity = 0;
		i = 0;
		while (i < d->heap.size)
		{
			x = d->heap.items[i++];
			if (x.coder_id != cid)
				heap_push(&tmp, x, sim->scheduler);
		}
		heap_destroy(&d->heap);
		d->heap = tmp;
	}
	pthread_mutex_unlock(&d->mutex);
	return (ok);
}

void	dongle_release(t_sim *sim, int did, int cid)
{
	t_dongle	*d;

	d = &sim->dongles[did];
	pthread_mutex_lock(&d->mutex);
	if (d->owner == cid)
	{
		d->owner = -1;
		d->available_at = now_ms() + sim->cooldown;
		pthread_cond_broadcast(&d->cond);
	}
	pthread_mutex_unlock(&d->mutex);
}
