#!/bin/sh
# Demo every bonus flag with CORRECT syntax (a number between flag and host).
# Run as root from anywhere:   sudo sh tests/demo.sh [host]
DIR=$(dirname "$0")
BIN="$DIR/../ft_traceroute"
H="${1:-8.8.8.8}"

for opt in "" "-m 5" "-q 1" "-f 3 -m 6" "-N 4" "-p 40000"; do
	echo "==================================================================="
	echo "\$ sudo ./ft_traceroute $opt $H"
	echo "-------------------------------------------------------------------"
	$BIN $opt "$H"
	echo
done
