#include "codexion.h"

static int init_dongles(t_sim *s)
{
	int i;

	s->dongles = malloc(sizeof(*s->dongles) * s->count);
	if (!s->dongles)
		return (0);
	i = 0;
	while (i < s->count)
	{
		memset(&s->dongles[i], 0, sizeof(*s->dongles));
		if (pthread_mutex_init(&s->dongles[i].mutex, NULL))
			return (0);
		if (pthread_cond_init(&s->dongles[i].cond, NULL))
		{
			pthread_mutex_destroy(&s->dongles[i].mutex);
			return (0);
		}
		s->dongles[i].owner = -1;
		s->dongles[i].available_at = s->start;
		s->initialized_dongles++;
		i++;
	}
	return (1);
}

int init_sim(t_sim *s)
{
	int i;

	s->start = now_ms();
	if (pthread_mutex_init(&s->state_mutex, NULL))
		return (0);
	s->state_ready = 1;
	if (pthread_mutex_init(&s->log_mutex, NULL))
		return (0);
	s->log_ready = 1;
	if (pthread_mutex_init(&s->order_mutex, NULL))
		return (0);
	s->order_ready = 1;
	if (!init_dongles(s))
		return (0);
	s->coders = malloc(sizeof(*s->coders) * s->count);
	if (!s->coders)
		return (0);
	i = 0;
	while (i < s->count)
	{
		s->coders[i].id = i + 1;
		s->coders[i].left = i;
		s->coders[i].right = (i + 1) % s->count;
		s->coders[i].compiles = 0;
		s->coders[i].last_compile = s->start;
		s->coders[i].sim = s;
		i++;
	}
	return (1);
}

void destroy_sim(t_sim *s)
{
	int i;

	if (s->dongles)
	{
		i = 0;
		while (i < s->initialized_dongles)
		{
			heap_destroy(&s->dongles[i].heap);
			pthread_cond_destroy(&s->dongles[i].cond);
			pthread_mutex_destroy(&s->dongles[i].mutex);
			i++;
		}
		free(s->dongles);
	}
	free(s->coders);
	if (s->order_ready)
		pthread_mutex_destroy(&s->order_mutex);
	if (s->log_ready)
		pthread_mutex_destroy(&s->log_mutex);
	if (s->state_ready)
		pthread_mutex_destroy(&s->state_mutex);
}
