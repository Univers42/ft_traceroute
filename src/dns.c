#include "ft_traceroute.h"

/* Resolve ctx->target once (AF_INET) into ctx->dst and ctx->dst_str. This is
   the only DNS done: the target's FQDN for the header. Hops stay numeric. */
int	resolve_target(t_ctx *ctx)
{
	struct addrinfo	hints;
	struct addrinfo	*res;
	int				gai;

	ft_memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	gai = getaddrinfo(ctx->target, NULL, &hints, &res);
	if (gai != 0)
		return (error_msg(ctx->target, gai_strerror(gai)));
	ctx->dst = *(struct sockaddr_in *)res->ai_addr;
	freeaddrinfo(res);
	ft_strlcpy(ctx->dst_str, inet_ntoa(ctx->dst.sin_addr), sizeof ctx->dst_str);
	return (0);
}
