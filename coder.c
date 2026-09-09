#include "codexion.h"

static void	sleep_ms(t_sim *sim, long duration)
{
	long	end;

	end = now_ms() + duration;
	while (!sim_stopped(sim) && now_ms() < end)
		usleep(1000);
}

static long	get_deadline(t_coder *c)
{
	long	deadline;

	pthread_mutex_lock(&c->sim->state_mutex);
	deadline = c->last_compile_start + c->sim->burnout;
	pthread_mutex_unlock(&c->sim->state_mutex);
	return (deadline);
}

static int	try_compile(t_coder *c)
{
	t_sim	*s;
	int		first;
	int		second;
	long	deadline;

	s = c->sim;
	first = c->left;
	second = c->right;

	if (first == second)
		return (0);

	if (first > second)
	{
		first = c->right;
		second = c->left;
	}

	deadline = get_deadline(c);
	if (!dongles_acquire(s, first, second, c->id, deadline))
		return (0);

	if (sim_stopped(s))
	{
		dongle_release(s, second, c->id);
		dongle_release(s, first, c->id);
		return (0);
	}

	pthread_mutex_lock(&s->state_mutex);
	c->last_compile_start = now_ms();
	pthread_mutex_unlock(&s->state_mutex);

	log_action(s, c->id, "is compiling");
	sleep_ms(s, s->compile);

	dongle_release(s, second, c->id);
	dongle_release(s, first, c->id);

	pthread_mutex_lock(&s->state_mutex);
	c->compiles++;
	if (c->compiles >= s->required)
	{
		int	i;
		int	all_done;

		i = 0;
		all_done = 1;
		while (i < s->count)
		{
			if (s->coders[i].compiles < s->required)
				all_done = 0;
			i++;
		}
		if (all_done)
			s->stop = 1;
	}
	pthread_mutex_unlock(&s->state_mutex);

	pthread_mutex_lock(&s->resource_mutex);
	pthread_cond_broadcast(&s->resource_cond);
	pthread_mutex_unlock(&s->resource_mutex);

	return (1);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;
	t_sim	*s;

	c = arg;
	s = c->sim;
	while (!sim_stopped(s))
	{
		if (!try_compile(c))
			break ;
		if (sim_stopped(s))
			break ;
		log_action(s, c->id, "is debugging");
		sleep_ms(s, s->debug);
		if (sim_stopped(s))
			break ;
		log_action(s, c->id, "is refactoring");
		sleep_ms(s, s->refactor);
	}
	return (NULL);
}
