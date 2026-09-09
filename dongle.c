#include "codexion.h"

static void remove_request(t_dongle *d, int id, t_scheduler policy)
{
	t_heap tmp;
	t_request r;
	int i;

	tmp.items = NULL;
	tmp.size = 0;
	tmp.capacity = 0;
	i = 0;
	while (i < d->heap.size)
	{
		r = d->heap.items[i++];
		if (r.id != id)
			heap_push(&tmp, r, policy);
	}
	heap_destroy(&d->heap);
	d->heap = tmp;
}

int take_dongle(t_sim *s, int did, int id, long deadline)
{
	t_dongle *d;
	t_request r;
	struct timespec ts;
	int ok;

	d = &s->dongles[did];
	pthread_mutex_lock(&s->order_mutex);
	r.id = id;
	r.order = ++s->order;
	r.deadline = deadline;
	pthread_mutex_unlock(&s->order_mutex);
	pthread_mutex_lock(&d->mutex);
	if (!heap_push(&d->heap, r, s->scheduler))
	{
		pthread_mutex_unlock(&d->mutex);
		return (0);
	}
	while (!is_stopped(s))
	{
		if (d->heap.size && d->heap.items[0].id == id
			&& d->owner == -1 && now_ms() >= d->available_at)
		{
			heap_pop(&d->heap, &r, s->scheduler);
			d->owner = id;
			pthread_mutex_unlock(&d->mutex);
			log_action(s, id, "has taken a dongle");
			return (1);
		}
		{
			struct timeval tv;

			gettimeofday(&tv, NULL);
			ts.tv_sec = tv.tv_sec;
			ts.tv_nsec = tv.tv_usec * 1000L + 2000000L;
			if (ts.tv_nsec >= 1000000000L)
			{
				ts.tv_sec++;
				ts.tv_nsec -= 1000000000L;
			}
		}
		pthread_cond_timedwait(&d->cond, &d->mutex, &ts);
	}
	ok = 0;
	remove_request(d, id, s->scheduler);
	pthread_mutex_unlock(&d->mutex);
	return (ok);
}

void put_dongle(t_sim *s, int did, int id)
{
	t_dongle *d;

	d = &s->dongles[did];
	pthread_mutex_lock(&d->mutex);
	if (d->owner == id)
	{
		d->owner = -1;
		d->available_at = now_ms() + s->cooldown;
		pthread_cond_broadcast(&d->cond);
	}
	pthread_mutex_unlock(&d->mutex);
}
