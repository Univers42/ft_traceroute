#include "ft_traceroute.h"

typedef struct s_trace
{
	t_slot		*slots;
	long long	*hop_min;    /* fastest RTT seen per hop, -1 = none yet */
	int			nqueries;
	int			max_hops;
	int			window;
	int			dest_hop;
	int			next_seq;
	int			in_flight;
	int			resolved;
}	t_trace;

static long long	tv_us(const struct timeval *t)
{
	return ((long long)t->tv_sec * 1000000LL + (long long)t->tv_usec);
}

/* Adaptive per-probe timeout (us): HERE x this hop's RTT once it has answered,
   else NEAR x the nearest answered hop's RTT, else the MAX cap; floored. */
static long long	probe_wait_us(const t_trace *st, int hop)
{
	long long	w;
	int			d;

	if (st->hop_min[hop - 1] >= 0)
		w = (long long)WAIT_HERE * st->hop_min[hop - 1];
	else
	{
		w = -1;
		d = 1;
		while (d < st->max_hops && w < 0)
		{
			if (hop - 1 - d >= 0 && st->hop_min[hop - 1 - d] >= 0)
				w = (long long)WAIT_NEAR * st->hop_min[hop - 1 - d];
			else if (hop - 1 + d < st->max_hops && st->hop_min[hop - 1 + d] >= 0)
				w = (long long)WAIT_NEAR * st->hop_min[hop - 1 + d];
			d++;
		}
		if (w < 0)
			w = WAIT_MAX_US;
	}
	if (w > WAIT_MAX_US)
		w = WAIT_MAX_US;
	if (w < WAIT_FLOOR_US)
		w = WAIT_FLOOR_US;
	return (w);
}

static int	hop_done(const t_trace *st, int hop)
{
	int	base;
	int	i;

	base = (hop - 1) * st->nqueries;
	i = 0;
	while (i < st->nqueries)
	{
		if (!st->slots[base + i].done)
			return (0);
		i++;
	}
	return (1);
}

static void	advance(t_trace *st)
{
	while (st->resolved < st->dest_hop && hop_done(st, st->resolved + 1))
		st->resolved++;
}

/* Earliest adaptive deadline among in-flight probes (hops <= dest_hop). */
static int	next_deadline(const t_trace *st, struct timeval *out)
{
	long long	best;
	long long	dl;
	int			found;
	int			i;

	found = 0;
	best = 0;
	i = 0;
	while (i < st->dest_hop * st->nqueries)
	{
		if (st->slots[i].sent_flag && !st->slots[i].done)
		{
			dl = tv_us(&st->slots[i].sent) + probe_wait_us(st, i / st->nqueries + 1);
			if (!found || dl < best)
			{
				best = dl;
				found = 1;
			}
		}
		i++;
	}
	out->tv_sec = (time_t)(best / 1000000LL);
	out->tv_usec = (suseconds_t)(best % 1000000LL);
	return (found);
}

static void	collect(t_ctx *ctx, t_trace *st)
{
	struct sockaddr_in	from;
	struct timeval		now;
	long long			rtt;
	int					seq;
	int					final;

	if (!read_reply(ctx, &seq, &from, &final))
		return ;
	if (seq >= st->max_hops * st->nqueries
		|| !st->slots[seq].sent_flag || st->slots[seq].done)
		return ;
	gettimeofday(&now, NULL);
	st->slots[seq].done = 1;
	st->slots[seq].r.got = 1;
	st->slots[seq].r.final = final;
	st->slots[seq].r.from = from;
	st->slots[seq].r.rtt_ms = tv_diff_ms(&st->slots[seq].sent, &now);
	st->in_flight--;
	rtt = tv_us(&now) - tv_us(&st->slots[seq].sent);
	if (st->hop_min[seq / st->nqueries] < 0 || rtt < st->hop_min[seq / st->nqueries])
		st->hop_min[seq / st->nqueries] = rtt;
	if (final && seq / st->nqueries + 1 < st->dest_hop)
		st->dest_hop = seq / st->nqueries + 1;
}

static void	expire(t_trace *st)
{
	struct timeval	now;
	long long		now_us;
	int				i;

	gettimeofday(&now, NULL);
	now_us = tv_us(&now);
	i = 0;
	while (i < st->next_seq)
	{
		if (st->slots[i].sent_flag && !st->slots[i].done && now_us
			- tv_us(&st->slots[i].sent) >= probe_wait_us(st, i / st->nqueries + 1))
		{
			st->slots[i].done = 1;
			st->slots[i].r.got = 0;
			st->in_flight--;
		}
		i++;
	}
}

static int	send_window(t_ctx *ctx, t_trace *st)
{
	while (st->in_flight < st->window && st->next_seq < st->dest_hop * st->nqueries)
	{
		if (send_probe(ctx, st->next_seq / st->nqueries + 1, st->next_seq) < 0)
			return (-1);
		gettimeofday(&st->slots[st->next_seq].sent, NULL);
		st->slots[st->next_seq].sent_flag = 1;
		st->in_flight++;
		st->next_seq++;
	}
	return (0);
}

/* Print hops first_ttl..dest_hop. No reverse DNS (subject: numeric jumps). */
static void	print_all(const t_ctx *ctx, const t_trace *st)
{
	t_reply	reps[MAX_NQUERIES];
	int		hop;
	int		base;
	int		i;

	hop = ctx->first_ttl;
	while (hop <= st->dest_hop)
	{
		base = (hop - 1) * st->nqueries;
		i = 0;
		while (i < st->nqueries)
		{
			reps[i] = st->slots[base + i].r;
			i++;
		}
		print_hop(hop, reps, st->nqueries);
		hop++;
	}
}

static int	trace_init(t_ctx *ctx, t_trace *st)
{
	int	total;
	int	i;

	total = ctx->max_hops * ctx->nqueries;
	st->slots = malloc(sizeof(t_slot) * (size_t)total);
	st->hop_min = malloc(sizeof(long long) * (size_t)ctx->max_hops);
	if (!st->slots || !st->hop_min)
	{
		free(st->slots);
		free(st->hop_min);
		error_msg("malloc", "out of memory");
		return (-1);
	}
	ft_memset(st->slots, 0, sizeof(t_slot) * (size_t)total);
	st->nqueries = ctx->nqueries;
	st->max_hops = ctx->max_hops;
	st->window = ctx->window;
	st->dest_hop = ctx->max_hops;
	st->next_seq = (ctx->first_ttl - 1) * ctx->nqueries;
	st->in_flight = 0;
	st->resolved = ctx->first_ttl - 1;
	i = 0;
	while (i < ctx->max_hops)
		st->hop_min[i++] = -1;
	return (0);
}

/* Windowed probe loop: keep up to window probes in flight, match replies by
   sequence, give up on silent probes adaptively, until the destination. */
static int	run(t_ctx *ctx)
{
	t_trace			st;
	struct timeval	deadline;
	int				rc;

	if (trace_init(ctx, &st) < 0)
		return (1);
	print_header(ctx);
	rc = 0;
	while (st.resolved < st.dest_hop)
	{
		if (send_window(ctx, &st) < 0)
		{
			rc = 1;
			break ;
		}
		advance(&st);
		if (st.resolved >= st.dest_hop || !next_deadline(&st, &deadline))
			break ;
		if (wait_readable(ctx->recv_fd, &deadline))
			collect(ctx, &st);
		expire(&st);
	}
	if (rc == 0)
		print_all(ctx, &st);
	free(st.slots);
	free(st.hop_min);
	return (rc);
}

int	main(int ac, char **av)
{
	t_ctx	ctx;
	int		rc;

	ft_memset(&ctx, 0, sizeof ctx);
	ctx.send_fd = -1;
	ctx.recv_fd = -1;
	ctx.first_ttl = 1;
	ctx.max_hops = HOPS_MAX;
	ctx.nqueries = NPROBES;
	ctx.port = BASE_PORT;
	ctx.window = WINDOW;
	rc = parse_args(ac, av, &ctx);
	if (rc == ARGS_HELP)
		return (print_usage(STDOUT_FILENO), 0);
	if (rc == ARGS_ERR)
		return (print_usage(STDERR_FILENO), 1);
	if (resolve_target(&ctx) < 0)
		return (1);
	if (open_sockets(&ctx) < 0)
		return (1);
	rc = run(&ctx);
	close_sockets(&ctx);
	return (rc);
}
