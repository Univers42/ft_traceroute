#!/bin/sh
# CLI / argument / error-path edge cases for ft_traceroute. No root required.
BIN="${BIN:-./ft_traceroute}"
pass=0
fail=0

check() { # desc  expected_exit  actual_exit  must_contain  output
	_desc="$1"; _exp="$2"; _act="$3"; _needle="$4"; _out="$5"; _ok=1
	if [ "$_act" != "$_exp" ]; then
		_ok=0; echo "  FAIL [$_desc]: exit $_act, expected $_exp"
	fi
	if [ -n "$_needle" ]; then
		case "$_out" in
			*"$_needle"*) : ;;
			*) _ok=0; echo "  FAIL [$_desc]: output missing '$_needle'" ;;
		esac
	fi
	if [ "$_ok" -eq 1 ]; then pass=$((pass + 1)); echo "  ok   [$_desc]"
	else fail=$((fail + 1)); fi
}

echo "== CLI edge cases =="
out=$("$BIN" --help 2>&1);                    check "--help exits 0"        0 $? "Usage:"        "$out"
out=$("$BIN" 2>&1);                           check "no args errors"        1 $? "missing host"  "$out"
out=$("$BIN" host1 host2 2>&1);               check "two hosts errors"      1 $? "only one host" "$out"
out=$("$BIN" host1 host2 host3 2>&1);         check "three hosts errors"    1 $? "only one host" "$out"
out=$("$BIN" -z 2>&1);                        check "unknown short option"  1 $? "unknown option" "$out"
out=$("$BIN" --bogus 2>&1);                   check "unknown long option"   1 $? "unknown option" "$out"
out=$("$BIN" "" 2>&1);                        check "empty arg errors"      1 $? ""              "$out"
out=$("$BIN" 999.999.999.999 2>&1);           check "invalid IP errors"     1 $? ""              "$out"
out=$("$BIN" no.such.host.invalid.tld 2>&1);  check "bad host errors"       1 $? ""              "$out"
out=$("$BIN" --help 8.8.8.8 2>&1);            check "--help wins (before)"  0 $? "Usage:"        "$out"
out=$("$BIN" 8.8.8.8 --help 2>&1);            check "--help wins (after)"   0 $? "Usage:"        "$out"

echo "== bonus flags: parsing / validation =="
out=$("$BIN" -m abc 8.8.8.8 2>&1);            check "-m non-numeric"        1 $? "needs a number" "$out"
out=$("$BIN" -N 8.8.8.8 2>&1);                check "-N host-as-value"      1 $? "needs a number" "$out"
out=$("$BIN" -q 0 8.8.8.8 2>&1);              check "-q below range"        1 $? "out of range"   "$out"
out=$("$BIN" -q 999 8.8.8.8 2>&1);            check "-q above range"        1 $? "out of range"   "$out"
out=$("$BIN" -p 99999 8.8.8.8 2>&1);          check "-p above range"        1 $? "out of range"   "$out"
out=$("$BIN" -p -1 8.8.8.8 2>&1);             check "-p negative"           1 $? "needs a number" "$out"
out=$("$BIN" -m 2>&1);                        check "-m missing value"      1 $? "needs a number" "$out"
out=$("$BIN" -f 9 -m 3 8.8.8.8 2>&1);         check "-f exceeds -m"         1 $? "exceeds max"   "$out"
out=$("$BIN" -p 65000 -m 255 -q 64 h 2>&1);   check "-p+-m*-q overflow"     1 $? "port range"    "$out"
out=$("$BIN" --help -m 5 2>&1);               check "--help with flags"     0 $? "Usage:"        "$out"

if [ "$(id -u)" -ne 0 ]; then
	out=$("$BIN" 127.0.0.1 2>&1);                  check "IP no-priv message"   1 $? "privileges are required" "$out"
	out=$("$BIN" localhost 2>&1);                  check "host no-priv message" 1 $? "privileges are required" "$out"
	out=$("$BIN" -m 5 -q 2 -N 8 -p 40000 -f 1 127.0.0.1 2>&1); check "valid flags parse -> priv" 1 $? "privileges are required" "$out"
else
	echo "  skip [no-priv tests] (running as root)"
fi

echo "== CLI: $pass passed, $fail failed =="
[ "$fail" -eq 0 ]
