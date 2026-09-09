/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hadrider <hadrider@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 14:00:14 by hadrider          #+#    #+#             */
/*   Updated: 2026/09/09 14:00:15 by hadrider         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000L + tv.tv_usec / 1000L);
}

int	is_stopped(t_sim *s)
{
	int	stop;

	pthread_mutex_lock(&s->state_mutex);
	stop = s->stop;
	pthread_mutex_unlock(&s->state_mutex);
	return (stop);
}

void	wake_all(t_sim *s)
{
	pthread_cond_broadcast(&s->queue_cond);
}

void	print_usage(void)
{
	printf("Usage: ./codexion number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown scheduler\n");
}
