#include "codexion.h"

static void sleep_ms(t_sim *s, long ms)
{
	long end;

	end = now_ms() + ms;
	while (!is_stopped(s) && now_ms() < end)
		usleep(1000);
}

static long deadline(t_coder *c)
{
	long value;

	pthread_mutex_lock(&c->sim->state_mutex);
	value = c->last_compile + c->sim->burnout;
	pthread_mutex_unlock(&c->sim->state_mutex);
	return (value);
}

static int take_two(t_coder *c)
{
	t_sim *s;
	int first;
	int second;
	long dl;

	s = c->sim;
	first = c->left;
	second = c->right;
	if (c->id % 2 == 0)
	{
		first = c->right;
		second = c->left;
	}
	dl = deadline(c);
	if (first == second)
	{
		if (!take_dongle(s, first, c->id, dl))
			return (0);
		while (!is_stopped(s))
			usleep(1000);
		put_dongle(s, first, c->id);
		return (0);
	}
	if (!take_dongle(s, first, c->id, dl))
		return (0);
	if (!take_dongle(s, second, c->id, dl))
	{
		put_dongle(s, first, c->id);
		return (0);
	}
	if (is_stopped(s))
	{
		put_dongle(s, second, c->id);
		put_dongle(s, first, c->id);
		return (0);
	}
	pthread_mutex_lock(&s->state_mutex);
	c->last_compile = now_ms();
	pthread_mutex_unlock(&s->state_mutex);
	log_action(s, c->id, "is compiling");
	sleep_ms(s, s->compile);
	put_dongle(s, second, c->id);
	put_dongle(s, first, c->id);
	pthread_mutex_lock(&s->state_mutex);
	c->compiles++;
	pthread_mutex_unlock(&s->state_mutex);
	return (1);
}

void *coder_routine(void *arg)
{
	t_coder *c;
	t_sim *s;

	c = arg;
	s = c->sim;
	while (!is_stopped(s))
	{
		if (!take_two(c))
			break ;
		if (is_stopped(s))
			break ;
		log_action(s, c->id, "is debugging");
		sleep_ms(s, s->debug);
		if (is_stopped(s))
			break ;
		log_action(s, c->id, "is refactoring");
		sleep_ms(s, s->refactor);
	}
	return (NULL);
}
