#!/bin/sh
# Validate output format against the traceroute grammar (validate_format.awk).
# Good fixtures must conform; bad fixtures must be rejected (negative tests).
# With root + a host arg, also validates a live trace.
DIR=$(dirname "$0")
AWK="$DIR/validate_format.awk"
BIN="${BIN:-./ft_traceroute}"
tmp=$(mktemp)
pass=0
fail=0

echo "== Format: good fixtures (must conform) =="
for f in "$DIR"/fixtures/*.txt; do
	if awk -f "$AWK" "$f" >"$tmp" 2>&1; then
		pass=$((pass + 1)); printf "  ok   %-18s %s\n" "$(basename "$f")" "$(tail -1 "$tmp")"
	else
		fail=$((fail + 1)); printf "  FAIL %-18s\n" "$(basename "$f")"; sed 's/^/        /' "$tmp"
	fi
done

echo "== Format: bad fixtures (must be rejected) =="
for f in "$DIR"/fixtures/bad/*.txt; do
	if awk -f "$AWK" "$f" >/dev/null 2>&1; then
		fail=$((fail + 1)); printf "  FAIL %-22s (validator accepted bad input!)\n" "$(basename "$f")"
	else
		pass=$((pass + 1)); printf "  ok   %-22s (correctly rejected)\n" "$(basename "$f")"
	fi
done

if [ "$#" -ge 1 ] && [ "$(id -u)" -eq 0 ]; then
	echo "== Format: live trace ($1) =="
	if "$BIN" "$1" >"$tmp" 2>&1 && awk -f "$AWK" "$tmp"; then
		pass=$((pass + 1)); echo "  ok   live trace conforms"
	else
		fail=$((fail + 1)); echo "  FAIL live trace"; sed 's/^/        /' "$tmp"
	fi
else
	echo "== Live trace skipped (run: sudo BIN=./ft_traceroute sh $DIR/test_format.sh 8.8.8.8) =="
fi

rm -f "$tmp"
echo "== Format: $pass passed, $fail failed =="
[ "$fail" -eq 0 ]
