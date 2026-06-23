#!/bin/sh
# Run the whole suite. Pass a host to also run the live format test (as root).
DIR=$(dirname "$0")
sh "$DIR/test_cli.sh"; c=$?
echo
sh "$DIR/test_format.sh" "$@"; f=$?
echo
if [ "$c" -eq 0 ] && [ "$f" -eq 0 ]; then
	echo "===== ALL TESTS PASSED ====="; exit 0
else
	echo "===== SOME TESTS FAILED ====="; exit 1
fi
