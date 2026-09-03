#include "codexion.h"

static void	request_stop(t_sim *s)
{
	int	i;

	pthread_mutex_lock(&s->state_mutex);
	s->stop = 1;
	pthread_mutex_unlock(&s->state_mutex);
	i = 0;
	while (i < s->count)
		pthread_cond_broadcast(&s->dongles[i++].cond);
}

int	main(int argc, char **argv)
{
	t_sim	s;
	int	created;
	int	i;

	memset(&s, 0, sizeof(s));
	if (!parse_args(&s, argc, argv))
		return (1);
	if (!init_sim(&s))
	{
		fprintf(stderr, "Error: initialization failed.\n");
		destroy_sim(&s);
		return (1);
	}
	created = 0;
	while (created < s.count)
	{
		if (pthread_create(&s.coders[created].thread, NULL, coder_routine,
				&s.coders[created]) != 0)
			break ;
		created++;
	}
	if (created != s.count)
		request_stop(&s);
	else if (pthread_create(&s.monitor, NULL, monitor_routine, &s) == 0)
		s.monitor_started = 1;
	else
		request_stop(&s);
	i = 0;
	while (i < created)
		pthread_join(s.coders[i++].thread, NULL);
	if (s.monitor_started)
		pthread_join(s.monitor, NULL);
	destroy_sim(&s);
	return (0);
}
