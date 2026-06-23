#include "ft_traceroute.h"

/* Milliseconds elapsed between two timestamps. */
double	tv_diff_ms(const struct timeval *start, const struct timeval *end)
{
	double	ms;

	ms = (double)(end->tv_sec - start->tv_sec) * 1000.0;
	ms += (double)(end->tv_usec - start->tv_usec) / 1000.0;
	return (ms);
}

/* Print "ft_traceroute: <where>[: <detail>]" to stderr. Returns -1 so callers
   can `return (error_msg(...));`. */
int	error_msg(const char *where, const char *detail)
{
	if (detail)
		fprintf(stderr, "%s: %s: %s\n", PROG_NAME, where, detail);
	else
		fprintf(stderr, "%s: %s\n", PROG_NAME, where);
	return (-1);
}
