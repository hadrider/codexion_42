/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hadrider <hadrider@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 13:28:27 by hadrider          #+#    #+#             */
/*   Updated: 2026/09/09 13:28:28 by hadrider         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	stop_sim(t_sim *s, int id)
{
	int	print;

	print = 0;
	pthread_mutex_lock(&s->state_mutex);
	if (!s->stop)
	{
		s->stop = 1;
		if (id > 0)
			print = 1;
	}
	pthread_mutex_unlock(&s->state_mutex);
	if (print)
		log_action(s, id, "burned out");
	wake_all(s);
}

static int	check_coders(t_sim *s, long now)
{
	int	i;
	int	done;

	i = 0;
	done = 1;
	while (i < s->count)
	{
		pthread_mutex_lock(&s->state_mutex);
		if (now - s->coders[i].last_compile >= s->burnout)
		{
			pthread_mutex_unlock(&s->state_mutex);
			stop_sim(s, s->coders[i].id);
			return (-1);
		}
		if (s->coders[i].compiles < s->required)
			done = 0;
		pthread_mutex_unlock(&s->state_mutex);
		i++;
	}
	return (done);
}

void	*monitor_routine(void *arg)
{
	t_sim	*s;
	int		done;

	s = arg;
	while (!is_stopped(s))
	{
		done = check_coders(s, now_ms());
		if (done < 0)
			return (NULL);
		if (done)
		{
			stop_sim(s, 0);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
