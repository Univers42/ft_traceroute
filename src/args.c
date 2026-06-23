#include "ft_traceroute.h"

typedef struct s_flag
{
	const char	*name;
	int			*dst;
	int			lo;
	int			hi;
}	t_flag;

void	print_usage(int fd)
{
	const char	*usage =
		"Usage:\n"
		"  " PROG_NAME " [options] <host>\n"
		"\n"
		"  <host>     IPv4 address or hostname to trace.\n"
		"  --help     Show this help and exit.\n"
		"  -f <n>     First TTL / start hop (default 1).\n"
		"  -m <n>     Max hops (default 30).\n"
		"  -q <n>     Probes per hop (default 3).\n"
		"  -p <port>  Base destination UDP port (default 33434).\n"
		"  -N <n>     Probes sent simultaneously (default 16).\n";

	write(fd, usage, ft_strlen(usage));
}

static int	parse_uint(const char *s, int *out)
{
	long	n;
	int		i;

	if (!s[0])
		return (-1);
	n = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (-1);
		n = n * 10 + (s[i] - '0');
		if (n > 2147483647L)
			return (-1);
		i++;
	}
	*out = (int)n;
	return (0);
}

static void	init_flags(t_flag *fl, t_ctx *ctx)
{
	fl[0] = (t_flag){"-f", &ctx->first_ttl, 1, 255};
	fl[1] = (t_flag){"-m", &ctx->max_hops, 1, 255};
	fl[2] = (t_flag){"-q", &ctx->nqueries, 1, MAX_NQUERIES};
	fl[3] = (t_flag){"-p", &ctx->port, 1, 65535};
	fl[4] = (t_flag){"-N", &ctx->window, 1, 256};
}

/* For the common mistake of writing the host where the value goes:
   "ft_traceroute: -N: needs a number before the host (e.g. -N 5 <host>)". */
static int	need_value(const char *flag)
{
	char	msg[64];

	ft_strlcpy(msg, "needs a number before the host (e.g. ", sizeof msg);
	ft_strlcat(msg, flag, sizeof msg);
	ft_strlcat(msg, " 5 <host>)", sizeof msg);
	return (error_msg(flag, msg));
}

static int	bad_range(const char *flag, const char *val)
{
	char	msg[40];

	ft_strlcpy(msg, "value out of range: ", sizeof msg);
	ft_strlcat(msg, val, sizeof msg);
	return (error_msg(flag, msg));
}

/* 1 = applied, 0 = not a flag, -1 = error (message already printed). */
static int	apply_flag(t_flag *fl, char **av, int *i, int ac)
{
	int	f;
	int	v;

	f = 0;
	while (f < 5)
	{
		if (ft_strcmp(av[*i], fl[f].name) == 0)
		{
			if (*i + 1 >= ac || parse_uint(av[*i + 1], &v) != 0)
				return (need_value(fl[f].name));
			if (v < fl[f].lo || v > fl[f].hi)
				return (bad_range(fl[f].name, av[*i + 1]));
			*fl[f].dst = v;
			*i += 2;
			return (1);
		}
		f++;
	}
	return (0);
}

int	parse_args(int ac, char **av, t_ctx *ctx)
{
	t_flag	fl[5];
	int		i;
	int		r;

	init_flags(fl, ctx);
	i = 1;
	while (i < ac)
	{
		if (ft_strcmp(av[i], "--help") == 0)
			return (ARGS_HELP);
		r = apply_flag(fl, av, &i, ac);
		if (r == -1)
			return (ARGS_ERR);
		if (r == 1)
			continue ;
		if (av[i][0] == '-' && av[i][1] != '\0')
			return (error_msg("unknown option", av[i]), ARGS_ERR);
		if (ctx->target != NULL)
			return (error_msg("only one host operand is allowed", av[i]), ARGS_ERR);
		ctx->target = av[i++];
	}
	if (ctx->target == NULL)
		return (error_msg("missing host operand", NULL), ARGS_ERR);
	if (ctx->first_ttl > ctx->max_hops)
		return (error_msg("first ttl (-f) exceeds max hops (-m)", NULL), ARGS_ERR);
	if (ctx->port + ctx->max_hops * ctx->nqueries > 65536)
		return (error_msg("port range (-p + -m*-q) exceeds 65535", NULL), ARGS_ERR);
	return (ARGS_OK);
}
