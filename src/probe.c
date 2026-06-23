#include "ft_traceroute.h"

/* Send probe number `seq` at the given TTL. The destination port (base + seq)
   encodes seq so the triggered ICMP error can be matched back to it. */
int	send_probe(t_ctx *ctx, int ttl, int seq)
{
	char	payload[DATA_LEN];
	ssize_t	sent;

	if (set_ttl(ctx->send_fd, ttl) < 0)
		return (-1);
	ft_memset(payload, 0, sizeof payload);
	ctx->dst.sin_port = htons((uint16_t)(ctx->port + seq));
	sent = sendto(ctx->send_fd, payload, sizeof payload, 0,
			(struct sockaddr *)&ctx->dst, sizeof ctx->dst);
	if (sent < 0)
		return (error_msg("sendto", strerror(errno)));
	return (0);
}
