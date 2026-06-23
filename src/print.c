#include "ft_traceroute.h"

void	print_header(const t_ctx *ctx)
{
	printf("traceroute to %s (%s), %d hops max, %d byte packets\n",
		ctx->target, ctx->dst_str, ctx->max_hops, PROBE_LEN);
}

/* One hop line, matching `traceroute -n`: "%2d" hop number, then per probe the
   responder IP (printed only when it changes) and the RTT, or "*" on timeout.
   No reverse DNS — hops stay numeric (subject: no DNS in the jump display). */
void	print_hop(int ttl, const t_reply *replies, int n)
{
	struct in_addr	last;
	int				has_last;
	int				i;

	printf("%2d ", ttl);
	has_last = 0;
	last.s_addr = 0;
	i = 0;
	while (i < n)
	{
		if (!replies[i].got)
			printf(" *");
		else
		{
			if (!has_last || replies[i].from.sin_addr.s_addr != last.s_addr)
			{
				printf(" %s", inet_ntoa(replies[i].from.sin_addr));
				last = replies[i].from.sin_addr;
				has_last = 1;
			}
			printf("  %.3f ms", replies[i].rtt_ms);
		}
		i++;
	}
	printf("\n");
}
