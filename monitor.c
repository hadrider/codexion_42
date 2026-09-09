#include "codexion.h"

static void	stop_sim(t_sim *s, int burnout_id)
{
	int	i;

	pthread_mutex_lock(&s->state_mutex);
	if (!s->stop)
	{
		s->stop = 1;
		if (burnout_id >= 0)
		{
			pthread_mutex_unlock(&s->state_mutex);
			log_action(s, burnout_id, "burned out");
			pthread_mutex_lock(&s->state_mutex);
		}
	}
	pthread_mutex_unlock(&s->state_mutex);
	i = 0;
	while (i < s->count)
		pthread_cond_broadcast(&s->dongles[i++].cond);
}

void	*monitor_routine(void *arg)
{
	t_sim	*s;
	long	now;
	int		i;
	int		all_done;

	s = arg;
	while (!sim_stopped(s))
	{
		now = now_ms();
		i = 0;
		all_done = 1;
		while (i < s->count)
		{
			pthread_mutex_lock(&s->state_mutex);
			if (s->coders[i].compiles > 0
				&& now - s->coders[i].last_compile_start >= s->burnout)
			{
				pthread_mutex_unlock(&s->state_mutex);
				stop_sim(s, s->coders[i].id);
				return (NULL);
			}
			if (s->coders[i].compiles < s->required)
				all_done = 0;
			pthread_mutex_unlock(&s->state_mutex);
			i++;
		}
		if (all_done)
		{
			stop_sim(s, -1);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}