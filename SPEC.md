# ft_traceroute — Specification (mandatory part)

**Status:** Approved · UDP probe mode (chosen 2026-06-23). Every line of code traces to a requirement here (spec-driven-workflow). Constraints: `en.subject.txt` + `CLAUDE.md`; the allowed-function whitelist is fixed by the subject.

## Context
Recode `traceroute` in C for the 42 mandatory part: discover each IPv4 hop to a target by sending UDP probes with increasing TTL and reading the ICMP errors they trigger. Graded by humans on Linux ≥ 4.0, diffed against the real `traceroute` (whose default is UDP mode).

## Functional requirements (MUST)
- **FR-1** CLI `ft_traceroute [--help] <host>`; `<host>` is one IPv4 dotted-quad **or** hostname.
- **FR-2** `--help` prints usage to stdout, exit 0. No other option in the mandatory part.
- **FR-3** Resolve `<host>` once via `getaddrinfo` (AF_INET). Failure → error to stderr, exit ≠ 0.
- **FR-4** Probe = UDP datagram; destination port starts 33434 and increments per probe; TTL = 1..MAX via `setsockopt(IP_TTL)`.
- **FR-5** Receive ICMP on a raw socket; classify type 11 (Time Exceeded) = intermediate hop, type 3 code 3 (Port Unreachable) = destination reached (stop after that hop).
- **FR-6** 3 probes per hop, 30 hops max; stop at destination or HOPS_MAX.
- **FR-7** Match each ICMP reply to its probe via the **embedded UDP destination port**.
- **FR-8** Per hop print: hop number, responder IP (numeric, shown once per distinct address), each probe RTT in ms; `*` for a probe with no reply within the wait window.
- **FR-9** Hops are shown **numerically — no reverse DNS in the jump display** (per the subject), which also keeps the receive path fast. The target FQDN still appears in the header.
- **FR-10** Header + hop-line format and indentation match the real `traceroute`.

## Non-functional requirements
- **NFR-1** Never crash; every syscall failure handled.
- **NFR-2** RTT within ±30 ms of real traceroute (`gettimeofday` bracketing each probe).
- **NFR-3** Compiles clean with `-Wall -Wextra -Werror`; no leaks (`freeaddrinfo`, `close`).
- **NFR-4** Only subject-allowed functions; no `fcntl`/`poll`/`ppoll` — `select` only.
- **NFR-5** Modular: `args/net/dns/probe/recv/print/util` behind header contracts.

## Acceptance criteria (Given / When / Then)
- **AC-1** Given `--help`, When run, Then usage prints, exit 0.
- **AC-2** Given no arg or >1 host, When run, Then usage error to stderr, exit ≠ 0.
- **AC-3** Given an invalid hostname, When run, Then a resolution error prints, exit ≠ 0, no crash.
- **AC-4** Given a reachable host (run as root), When traced, Then header + one line per hop print, ending at the destination, format matching real traceroute.
- **AC-5** Given a hop that does not answer within the wait, Then that probe prints `*`.
- **AC-6** Given run without privileges, Then it fails cleanly stating root / `CAP_NET_RAW` is required.

## Defaults
HOPS_MAX = 30 · NPROBES = 3 · BASE_PORT = 33434 · PROBE_LEN = 60 B (IP 20 + UDP 8 + data 32) · WAIT = 3 s.

## Bonus (implemented)
Five flags turn the trace constants into runtime options, parsed/validated without any non-allowed libc:
- `-f <n>` first TTL / start hop · `-m <n>` max hops · `-q <n>` probes per hop · `-p <port>` base UDP port · `-N <n>` probes in flight.
- Bounds enforced (e.g. `1..255`, `1..65535`); cross-checks `-f ≤ -m` and `-p + -m·-q ≤ 65535`. Slot storage is `malloc`-sized to `-m·-q`.

## Out of scope
IPv6; ICMP-echo (`-I`) mode — the probe layer stays separable so it could be added without reworking callers; reverse-DNS name display (`-d`).
