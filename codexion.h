#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

typedef enum e_scheduler
{
	SCHED_POLICY_FIFO,
	SCHED_POLICY_EDF
}	t_scheduler;

typedef struct s_request
{
	int				coder_id;
	unsigned long	arrival;
	long			deadline;
}	t_request;

typedef struct s_heap
{
	t_request		*items;
	int				size;
	int				capacity;
}	t_heap;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	long			available_at;
	t_heap			heap;
	int				owner;
}	t_dongle;

typedef struct s_sim	t_sim;

typedef struct s_coder
{
	int				id;
	int				left;
	int				right;
	int				compiles;
	long			last_compile_start;
	pthread_t		thread;
	t_sim			*sim;
}	t_coder;

typedef struct s_sim
{
	int				count;
	long			burnout;
	long			compile;
	long			debug;
	long			refactor;
	int				required;
	long			cooldown;
	t_scheduler		scheduler;
	long			start_ms;
	int				stop;
	int				initialized_dongles;
	int				initialized_coders;
	int				state_mutex_ready;
	int				log_mutex_ready;
	int				seq_mutex_ready;
	int				resource_mutex_ready;
	int				resource_cond_ready;
	pthread_mutex_t	state_mutex;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	seq_mutex;
	pthread_mutex_t	resource_mutex;
	pthread_cond_t	resource_cond;
	unsigned long	arrival_seq;
	t_dongle		*dongles;
	t_coder			*coders;
	pthread_t		monitor;
	int				monitor_started;
}	t_sim;

int		parse_args(t_sim *sim, int argc, char **argv);
void	print_usage(void);

int		init_sim(t_sim *sim);
void	destroy_sim(t_sim *sim);

long	now_ms(void);
int		sim_stopped(t_sim *sim);

void	request_init(t_request *r, int id, unsigned long seq, long deadline);

int		heap_push(t_heap *h, t_request r, t_scheduler s);
int		heap_pop(t_heap *h, t_request *r, t_scheduler s);
void	heap_destroy(t_heap *h);

int		dongles_acquire(t_sim *sim, int first, int second,
			int cid, long deadline);
void	dongle_release(t_sim *sim, int did, int cid);

void	log_action(t_sim *sim, int cid, const char *action);

void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);

#endif
