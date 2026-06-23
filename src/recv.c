#include "ft_traceroute.h"

/* Recover the probe sequence from the UDP dest port embedded in an ICMP
   error's quoted datagram (original IP header + UDP header). -1 if not ours. */
static int	embedded_seq(const uint8_t *orig, size_t len, int base, int total)
{
	const struct iphdr	*ip;
	const struct udphdr	*udp;
	size_t				ip_hl;
	int					port;

	if (len < sizeof(struct iphdr))
		return (-1);
	ip = (const struct iphdr *)orig;
	ip_hl = (size_t)ip->ihl * 4;
	if (len < ip_hl + sizeof(struct udphdr))
		return (-1);
	udp = (const struct udphdr *)(orig + ip_hl);
	port = (int)ntohs(udp->dest);
	if (port < base || port >= base + total)
		return (-1);
	return (port - base);
}

/* Read one raw ICMP packet. On a valid probe reply, fill the seq, from and
   final outputs and return 1; otherwise return 0 (not ours / not parseable). */
int	read_reply(t_ctx *ctx, int *seq, struct sockaddr_in *from, int *final)
{
	uint8_t					buf[1024];
	const struct iphdr		*outer;
	const struct icmphdr	*icmp;
	ssize_t					n;
	size_t					hl;
	int						s;

	n = recvfrom(ctx->recv_fd, buf, sizeof buf, 0, NULL, NULL);
	if (n < (ssize_t)sizeof(struct iphdr))
		return (0);
	outer = (const struct iphdr *)buf;
	hl = (size_t)outer->ihl * 4;
	if ((size_t)n < hl + sizeof(struct icmphdr))
		return (0);
	icmp = (const struct icmphdr *)(buf + hl);
	if (icmp->type != ICMP_TIME_EXCEEDED && icmp->type != ICMP_DEST_UNREACH)
		return (0);
	s = embedded_seq(buf + hl + sizeof(struct icmphdr),
			(size_t)n - hl - sizeof(struct icmphdr), ctx->port,
			ctx->max_hops * ctx->nqueries);
	if (s < 0)
		return (0);
	*seq = s;
	ft_memset(from, 0, sizeof(*from));
	from->sin_family = AF_INET;
	from->sin_addr.s_addr = outer->saddr;
	*final = (icmp->type == ICMP_DEST_UNREACH && icmp->code == ICMP_PORT_UNREACH);
	return (1);
}

/* select() on fd until *deadline. Returns 1 if readable, 0 on timeout/error. */
int	wait_readable(int fd, const struct timeval *deadline)
{
	fd_set			rfds;
	struct timeval	now;
	struct timeval	tv;
	long			usec;

	gettimeofday(&now, NULL);
	usec = (deadline->tv_sec - now.tv_sec) * 1000000L
		+ (deadline->tv_usec - now.tv_usec);
	if (usec < 0)
		usec = 0;
	tv.tv_sec = usec / 1000000L;
	tv.tv_usec = usec % 1000000L;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	if (select(fd + 1, &rfds, NULL, NULL, &tv) <= 0)
		return (0);
	return (1);
}
