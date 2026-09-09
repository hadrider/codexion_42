/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hadrider <hadrider@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 13:29:25 by hadrider          #+#    #+#             */
/*   Updated: 2026/09/09 13:29:26 by hadrider         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	stop_sim(t_sim *s)
{
	pthread_mutex_lock(&s->state_mutex);
	s->stop = 1;
	pthread_mutex_unlock(&s->state_mutex);
	wake_all(s);
}

static int	create_coders(t_sim *s)
{
	int	i;

	i = 0;
	while (i < s->count)
	{
		if (pthread_create(&s->coders[i].thread, NULL,
				coder_routine, &s->coders[i]))
			return (i);
		i++;
	}
	return (i);
}

static void	join_coders(t_sim *s, int count)
{
	int	i;

	i = 0;
	while (i < count)
		pthread_join(s->coders[i++].thread, NULL);
}

int	main(int argc, char **argv)
{
	t_sim	s;
	int		created;

	memset(&s, 0, sizeof(s));
	if (!parse_args(&s, argc, argv) || !init_sim(&s))
	{
		if (argc == 9)
			destroy_sim(&s);
		return (1);
	}
	created = create_coders(&s);
	if (created != s.count)
		stop_sim(&s);
	else if (!pthread_create(&s.monitor, NULL, monitor_routine, &s))
		s.monitor_started = 1;
	else
		stop_sim(&s);
	join_coders(&s, created);
	if (s.monitor_started)
		pthread_join(s.monitor, NULL);
	destroy_sim(&s);
	return (0);
}
