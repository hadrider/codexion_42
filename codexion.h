#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

typedef enum e_scheduler
{
	FIFO,
	EDF
}t_scheduler;

typedef struct s_request
{
	int			id;
	unsigned long	order;
	long			deadline;
}t_request;

typedef struct s_heap
{
	t_request	*items;
	int		size;
	int		capacity;
}t_heap;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	t_heap			heap;
	long			available_at;
	int			owner;
}t_dongle;

typedef struct s_sim t_sim;

typedef struct s_coder
{
	int		id;
	int		left;
	int		right;
	int		compiles;
	long		last_compile;
	pthread_t	thread;
	t_sim		*sim;
}t_coder;

typedef struct s_sim
{
	int			count;
	long			burnout;
	long			compile;
	long			debug;
	long			refactor;
	int			required;
	long			cooldown;
	t_scheduler	scheduler;
	long			start;
	int			stop;
	unsigned long	order;
	pthread_mutex_t	state_mutex;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	order_mutex;
	int			state_ready;
	int			log_ready;
	int			order_ready;
	int			initialized_dongles;
	t_dongle	*dongles;
	t_coder		*coders;
	pthread_t	monitor;
	int			monitor_started;
}t_sim;

int		parse_args(t_sim *s, int argc, char **argv);
void		print_usage(void);
int		init_sim(t_sim *s);
void		destroy_sim(t_sim *s);
long		now_ms(void);
int		is_stopped(t_sim *s);
void		wake_all(t_sim *s);
int		heap_push(t_heap *h, t_request r, t_scheduler policy);
int		heap_pop(t_heap *h, t_request *r, t_scheduler policy);
void		heap_destroy(t_heap *h);
int		take_dongle(t_sim *s, int did, int id, long deadline);
void		put_dongle(t_sim *s, int did, int id);
void		log_action(t_sim *s, int id, const char *msg);
void		*coder_routine(void *arg);
void		*monitor_routine(void *arg);

#endif
