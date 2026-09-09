/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hadrider <hadrider@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 13:28:58 by hadrider          #+#    #+#             */
/*   Updated: 2026/09/09 13:28:59 by hadrider         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	add_num(char *buf, int pos, long n)
{
	char	tmp[20];
	int		i;

	i = 0;
	if (n == 0)
		buf[pos++] = '0';
	while (n > 0)
	{
		tmp[i++] = '0' + n % 10;
		n /= 10;
	}
	while (i > 0)
		buf[pos++] = tmp[--i];
	return (pos);
}

void	log_action(t_sim *s, int id, const char *msg)
{
	char	buf[128];
	int		i;
	int		len;

	len = add_num(buf, 0, now_ms() - s->start);
	buf[len++] = ' ';
	len = add_num(buf, len, id);
	buf[len++] = ' ';
	i = 0;
	while (msg[i])
		buf[len++] = msg[i++];
	buf[len++] = '\n';
	pthread_mutex_lock(&s->log_mutex);
	write(1, buf, len);
	pthread_mutex_unlock(&s->log_mutex);
}
