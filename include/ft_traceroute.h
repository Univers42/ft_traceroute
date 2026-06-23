/* ft_traceroute — module contracts (mandatory + bonus flags, UDP probe mode) */
#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdint.h>
# include <unistd.h>
# include <errno.h>
# include <sys/time.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <netinet/udp.h>
# include <arpa/inet.h>
# include <netdb.h>

# define PROG_NAME    "ft_traceroute"
# define HOPS_MAX     30      /* default for -m */
# define NPROBES      3       /* default for -q */
# define WINDOW       16      /* default for -N */
# define BASE_PORT    33434   /* default for -p */
# define MAX_NQUERIES 64      /* -q upper bound; bounds the print buffer */
# define PROBE_LEN    60      /* advertised IPv4 packet size: IP(20) + UDP(8) + data */
# define DATA_LEN     (PROBE_LEN - (int)sizeof(struct iphdr) - (int)sizeof(struct udphdr))

/* Adaptive per-probe timeout, like real traceroute: a silent hop gives up fast
   once neighbours have answered, instead of a flat multi-second wait. */
# define WAIT_MAX_US    5000000LL  /* hard cap per probe: 5 s */
# define WAIT_NEAR      10         /* dead hop: give up at NEAR x a nearby hop's RTT */
# define WAIT_HERE      3          /* more probes of an answered hop: HERE x its RTT */
# define WAIT_FLOOR_US  10000LL    /* never wait less than 10 ms */

# define ARGS_OK      0
# define ARGS_HELP    1
# define ARGS_ERR   (-1)

/* Run context: resolved destination, the two sockets, and the runtime config
   (defaults above, overridable by the bonus flags -f/-m/-q/-p/-N). */
typedef struct s_ctx
{
	const char			*target;
	char				dst_str[INET_ADDRSTRLEN];
	struct sockaddr_in	dst;
	int					send_fd;
	int					recv_fd;
	int					first_ttl;   /* -f */
	int					max_hops;    /* -m */
	int					nqueries;    /* -q */
	int					port;        /* -p */
	int					window;      /* -N */
}	t_ctx;

typedef struct s_reply
{
	int					got;
	int					final;
	struct sockaddr_in	from;
	double				rtt_ms;
}	t_reply;

typedef struct s_slot
{
	struct timeval	sent;
	int				sent_flag;
	int				done;
	t_reply			r;
}	t_slot;

/* args.c */
int		parse_args(int ac, char **av, t_ctx *ctx);
void	print_usage(int fd);

/* dns.c */
int		resolve_target(t_ctx *ctx);

/* net.c */
int		open_sockets(t_ctx *ctx);
void	close_sockets(t_ctx *ctx);
int		set_ttl(int fd, int ttl);

/* probe.c */
int		send_probe(t_ctx *ctx, int ttl, int seq);

/* recv.c */
int		wait_readable(int fd, const struct timeval *deadline);
int		read_reply(t_ctx *ctx, int *seq, struct sockaddr_in *from, int *final);

/* print.c */
void	print_header(const t_ctx *ctx);
void	print_hop(int ttl, const t_reply *replies, int n);

/* util.c */
double	tv_diff_ms(const struct timeval *start, const struct timeval *end);
int		error_msg(const char *where, const char *detail);

/* ft.c — libft replacements (subject grants only read/write/malloc/free) */
void	*ft_memset(void *b, int c, size_t len);
size_t	ft_strlen(const char *s);
int		ft_strcmp(const char *a, const char *b);
size_t	ft_strlcpy(char *dst, const char *src, size_t size);
size_t	ft_strlcat(char *dst, const char *src, size_t size);

#endif
