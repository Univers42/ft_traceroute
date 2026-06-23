#include "ft_traceroute.h"

int	set_ttl(int fd, int ttl)
{
	if (setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl, sizeof ttl) < 0)
		return (error_msg("setsockopt IP_TTL", strerror(errno)));
	return (0);
}

/* UDP send socket + raw ICMP receive socket. The raw socket needs root. */
int	open_sockets(t_ctx *ctx)
{
	ctx->send_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ctx->send_fd < 0)
		return (error_msg("socket (udp)", strerror(errno)));
	ctx->recv_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (ctx->recv_fd < 0)
	{
		close(ctx->send_fd);
		ctx->send_fd = -1;
		if (errno == EPERM || errno == EACCES)
			return (error_msg("raw socket",
					"root or CAP_NET_RAW privileges are required"));
		return (error_msg("socket (raw icmp)", strerror(errno)));
	}
	return (0);
}

void	close_sockets(t_ctx *ctx)
{
	if (ctx->send_fd >= 0)
		close(ctx->send_fd);
	if (ctx->recv_fd >= 0)
		close(ctx->recv_fd);
	ctx->send_fd = -1;
	ctx->recv_fd = -1;
}
