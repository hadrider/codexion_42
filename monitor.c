#include "codexion.h"

static void stop_sim(t_sim *s, int id)
{
	int print;

	print = 0;
	pthread_mutex_lock(&s->state_mutex);
	if (!s->stop)
	{
		s->stop = 1;
		print = id > 0;
	}
	pthread_mutex_unlock(&s->state_mutex);
	if (print)
		log_action(s, id, "burned out");
	wake_all(s);
}

void *monitor_routine(void *arg)
{
	t_sim *s;
	long now;
	int i;
	int done;

	s = arg;
	while (!is_stopped(s))
	{
		now = now_ms();
		done = 1;
		i = 0;
		while (i < s->count)
		{
			pthread_mutex_lock(&s->state_mutex);
			if (now - s->coders[i].last_compile >= s->burnout)
			{
				pthread_mutex_unlock(&s->state_mutex);
				stop_sim(s, s->coders[i].id);
				return (NULL);
			}
			if (s->coders[i].compiles < s->required)
				done = 0;
			pthread_mutex_unlock(&s->state_mutex);
			i++;
		}
		if (done)
		{
			stop_sim(s, 0);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
