#include "codexion.h"

static void stop_sim(t_sim *s)
{
	pthread_mutex_lock(&s->state_mutex);
	s->stop = 1;
	pthread_mutex_unlock(&s->state_mutex);
	wake_all(s);
}

int main(int argc, char **argv)
{
	t_sim s;
	int created;
	int i;

	memset(&s, 0, sizeof(s));
	if (!parse_args(&s, argc, argv) || !init_sim(&s))
	{
		if (argc == 9)
			destroy_sim(&s);
		return (1);
	}
	created = 0;
	while (created < s.count)
	{
		if (pthread_create(&s.coders[created].thread, NULL,
			coder_routine, &s.coders[created]))
			break ;
		created++;
	}
	if (created != s.count)
		stop_sim(&s);
	else if (!pthread_create(&s.monitor, NULL, monitor_routine, &s))
		s.monitor_started = 1;
	else
		stop_sim(&s);
	i = 0;
	while (i < created)
		pthread_join(s.coders[i++].thread, NULL);
	if (s.monitor_started)
		pthread_join(s.monitor, NULL);
	destroy_sim(&s);
	return (0);
}
