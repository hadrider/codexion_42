#include "codexion.h"

static int number(char *s, int *value)
{
	long n;
	int i;

	if (!s || !s[0])
		return (0);
	n = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		if (n > (2147483647L - (s[i] - '0')) / 10)
			return (0);
		n = n * 10 + s[i++] - '0';
	}
	if (n < 0)
		return (0);
	*value = (int)n;
	return (1);
}

int parse_args(t_sim *s, int argc, char **argv)
{
	int v[7];
	int i;

	if (argc != 9)
		return (print_usage(), 0);
	i = 0;
	while (i < 7)
	{
		if (!number(argv[i + 1], &v[i]))
			return (printf("Error: arguments must be positive integers.\n"), 0);
		i++;
	}
	if (strcmp(argv[8], "fifo") && strcmp(argv[8], "edf"))
		return (printf("Error: scheduler must be 'fifo' or 'edf'.\n"), 0);
	if (!strcmp(argv[8], "edf"))
		s->scheduler = EDF;
	else
		s->scheduler = FIFO;
	s->count = v[0];
	s->burnout = v[1];
	s->compile = v[2];
	s->debug = v[3];
	s->refactor = v[4];
	s->required = v[5];
	s->cooldown = v[6];
	return (1);
}
