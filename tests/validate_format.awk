# Validate numeric traceroute output ("traceroute -n" style) against the format
# grammar. Works on ft_traceroute and system `traceroute -n` output alike.
# Usage: awk -f validate_format.awk <output-file>
# Exit 0 if every line conforms, 1 otherwise (with per-line diagnostics).
BEGIN {
	fails = 0
	time  = "[0-9]+\\.[0-9][0-9][0-9] ms"          # 1.234 ms  (printf %.3f)
	ip    = "([0-9]+\\.){3}[0-9]+"                  # dotted-quad, no name
	# one probe: " *"  |  "  TIME" (same host)  |  " IP  TIME" (new host)
	probe = "( \\*|  " time "| " ip "  " time ")"
	# hop line: "%2d " then exactly three probes
	hop   = "^( [1-9]|[1-3][0-9]) " probe probe probe "$"
	head  = "^traceroute to .+ \\(([0-9]+\\.){3}[0-9]+\\), 30 hops max, 60 byte packets$"
}
NR == 1 {
	if ($0 !~ head) { printf("FAIL line %d (header): [%s]\n", NR, $0); fails++ }
	expect = 1
	next
}
{
	n = $1 + 0
	if (n != expect)   { printf("FAIL line %d: hop %d, expected %d\n", NR, n, expect); fails++ }
	if (n > 30)        { printf("FAIL line %d: hop %d exceeds 30\n", NR, n); fails++ }
	if ($0 !~ hop)     { printf("FAIL line %d (format): [%s]\n", NR, $0); fails++ }
	expect++
}
END {
	if (fails == 0) printf("OK: header + %d hop line(s) conform\n", NR - 1)
	else            printf("FAILED: %d issue(s)\n", fails)
	exit (fails == 0 ? 0 : 1)
}
