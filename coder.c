/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hadrider <hadrider@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 13:29:13 by hadrider          #+#    #+#             */
/*   Updated: 2026/09/09 13:29:14 by hadrider         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	sleep_ms(t_sim *s, long ms)
{
	long	end;

	end = now_ms() + ms;
	while (!is_stopped(s) && now_ms() < end)
		usleep(1000);
}

static long	deadline(t_coder *c)
{
	long	value;

	pthread_mutex_lock(&c->sim->state_mutex);
	value = c->last_compile + c->sim->burnout;
	pthread_mutex_unlock(&c->sim->state_mutex);
	return (value);
}

static int	finish_compile(t_coder *c)
{
	t_sim	*s;
	int		done;
	int		i;

	s = c->sim;
	pthread_mutex_lock(&s->state_mutex);
	c->compiles++;
	done = 1;
	i = 0;
	while (i < s->count)
	{
		if (s->coders[i].compiles < s->required)
			done = 0;
		i++;
	}
	if (done)
		s->stop = 1;
	pthread_mutex_unlock(&s->state_mutex);
	return (done);
}

static int	take_two(t_coder *c)
{
	t_sim	*s;
	long	dl;
	int		done;

	s = c->sim;
	dl = deadline(c);
	if (!take_dongles(s, c, dl))
		return (0);
	pthread_mutex_lock(&s->state_mutex);
	c->last_compile = now_ms();
	pthread_mutex_unlock(&s->state_mutex);
	log_action(s, c->id, "is compiling");
	sleep_ms(s, s->compile);
	put_dongle(s, c->left, c->id);
	put_dongle(s, c->right, c->id);
	done = finish_compile(c);
	if (done)
		wake_all(s);
	return (1);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;
	t_sim	*s;

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
