#include "codexion.h"

static int	parse_positive(char *s, int *value)
{
	long	n;
	int		i;

	if (!s || !s[0])
		return (0);
	n = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		n = n * 10 + (s[i] - '0');
		if (n > 2147483647L)
			return (0);
		i++;
	}
	if (n < 0)
		return (0);
	*value = (int)n;
	return (1);
}

int	parse_args(t_sim *sim, int argc, char **argv)
{
	int	v[7];
	int	i;

	if (argc != 9)
		return (print_usage(), 0);
	i = 0;
	while (i < 7)
	{
		if (!parse_positive(argv[i + 1], &v[i]))
			return (fprintf(stderr, "Error: numeric arguments must be positive "
					"integers.\n"), print_usage(), 0);
		i++;
	}
	if (strcmp(argv[8], "fifo") && strcmp(argv[8], "edf"))
		return (fprintf(stderr, "Error: scheduler must be 'fifo' or 'edf'.\n"),
			print_usage(), 0);
	sim->count = v[0];
	sim->burnout = v[1];
	sim->compile = v[2];
	sim->debug = v[3];
	sim->refactor = v[4];
	sim->required = v[5];
	sim->cooldown = v[6];
	sim->scheduler = (strcmp(argv[8], "edf") == 0)
		? SCHED_POLICY_EDF : SCHED_POLICY_FIFO;
	return (1);
}
