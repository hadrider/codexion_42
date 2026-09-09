#include "codexion.h"

static int	number_to_text(long n, char *buf, int pos)
{
	char	tmp[24];
	int	i;

	if (n == 0)
	{
		buf[pos++] = '0';
		return (pos);
	}
	i = 0;
	while (n > 0)
	{
		tmp[i++] = (char)('0' + (n % 10));
		n /= 10;
	}
	while (i > 0)
		buf[pos++] = tmp[--i];
	return (pos);
}

void	log_action(t_sim *sim, int cid, const char *action)
{
	char	buf[128];
	int	len;
	long	t;
	int	i;
	int	stopped;

	pthread_mutex_lock(&sim->log_mutex);
	pthread_mutex_lock(&sim->state_mutex);
	stopped = sim->stop;
	pthread_mutex_unlock(&sim->state_mutex);
	if (stopped && strcmp(action, "burned out") != 0)
	{
		pthread_mutex_unlock(&sim->log_mutex);
		return ;
	}
	t = now_ms() - sim->start_ms;
	len = 0;
	len = number_to_text(t, buf, len);
	buf[len++] = ' ';
	len = number_to_text(cid, buf, len);
	buf[len++] = ' ';
	i = 0;
	while (action[i])
		buf[len++] = action[i++];
	buf[len++] = '\n';
	write(1, buf, len);
	pthread_mutex_unlock(&sim->log_mutex);
}
