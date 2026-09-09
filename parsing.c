/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hadrider <hadrider@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 13:05:47 by hadrider          #+#    #+#             */
/*   Updated: 2026/09/09 13:28:13 by hadrider         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	number(char *str, int *value)
{
	long	n;
	int		i;

	if (!str || !str[0])
		return (0);
	n = 0;
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		if (n > (2147483647L - (str[i] - '0')) / 10)
			return (0);
		n = n * 10 + str[i++] - '0';
	}
	*value = (int)n;
	return (1);
}

static int	read_numbers(int *v, int argc, char **argv)
{
	int	i;

	if (argc != 9)
		return (0);
	i = 0;
	while (i < 7)
	{
		if (!number(argv[i + 1], &v[i]))
			return (0);
		i++;
	}
	return (1);
}

static void	set_values(t_sim *s, int *v)
{
	s->count = v[0];
	s->burnout = v[1];
	s->compile = v[2];
	s->debug = v[3];
	s->refactor = v[4];
	s->required = v[5];
	s->cooldown = v[6];
}

int	parse_args(t_sim *s, int argc, char **argv)
{
	int	v[7];

	if (!read_numbers(v, argc, argv))
	{
		if (argc != 9)
			print_usage();
		else
			printf("Error: arguments must be positive integers.\n");
		return (0);
	}
	if (strcmp(argv[8], "fifo") && strcmp(argv[8], "edf"))
	{
		printf("Error: scheduler must be 'fifo' or 'edf'.\n");
		return (0);
	}
	if (!strcmp(argv[8], "edf"))
		s->scheduler = EDF;
	else
		s->scheduler = FIFO;
	set_values(s, v);
	return (1);
}
