# Tests

Run everything: `make test`

- **CLI tests** (`test_cli.sh`) — args, `--help`, error paths, no-privilege message. No root.
- **Format tests** (`test_format.sh`) — validate output against the real-`traceroute`
  grammar (`validate_format.awk`) using captured fixtures, plus a live trace when run as root.

The fixtures in `fixtures/` are real captures: `ft_*.txt` from this program, `real_*.txt`
from the system `traceroute`. **Both must satisfy the same grammar** — that is the
format-parity proof (you can't byte-`diff` two live runs: RTTs and routes vary every time).

## Live format test (needs root)
The binary isn't on `PATH`, and `sudo` resets `PATH`, so always use `./`:

```sh
sudo BIN=./ft_traceroute sh tests/test_format.sh 8.8.8.8
```

## Compare against the real tool (structure, not bytes)
```sh
sudo apt install traceroute
sudo ./ft_traceroute  8.8.8.8 | awk -f tests/validate_format.awk   # must PASS
sudo    traceroute -n 8.8.8.8 | awk -f tests/validate_format.awk   # -n (numeric) -> same format
```
