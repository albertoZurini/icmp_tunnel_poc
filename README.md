# ICMP Tunnel POC

A proof-of-concept for **ICMP tunneling**: sending arbitrary data inside ICMP echo request/reply packets. Raw sockets are used to build and parse IP/ICMP headers and carry a small payload in the echo data. Useful for learning networking, encapsulation, and as a minimal tunnel demo (e.g. in environments where only ICMP is allowed).

## Requirements

- **Linux** is mandatory (raw sockets, `SOCK_RAW`, `IPPROTO_ICMP`). Before running the receiver, disable the kernel’s built-in ICMP echo replies so only this POC handles them:

  ```bash
  sudo sysctl -w net.ipv4.icmp_echo_ignore_all=1
  ```

  (Set to `0` to restore default behaviour when you’re done.)

- **Root** (or `CAP_NET_RAW`) to run the receiver and sender
- **C compiler** (e.g. GCC), `make`

## Build

```bash
make
```

Produces:

- `receive` — listens for ICMP echo requests and echoes back (with optional payload)
- `send` — sends ICMP echo requests to a host with an optional payload

## Usage

**Receiver** (run on the machine that will get the echo requests). First disable the kernel’s ICMP echo handler (see Requirements):

```bash
sudo sysctl -w net.ipv4.icmp_echo_ignore_all=1
sudo ./receive
```

Optionally bind to a specific interface:

```bash
sudo ./receive 192.168.1.100
```

**Sender** (run from another host):

```bash
sudo ./send <server_ip_or_hostname>
```

Example with a hostname:

```bash
sudo ./send my-server.example.com
```

Payload is carried in the ICMP echo data; maximum payload size is defined in `icmp_utils.h` (`ICMP_PAYLOAD_MAX`).

## Project layout

| Path            | Description                          |
|-----------------|--------------------------------------|
| `receive.c`     | Raw ICMP receiver (echo reply)       |
| `send.c`        | Raw ICMP sender (echo request)       |
| `icmp_utils.c`  | Shared helpers (checksum, parse/build) |
| `icmp_utils.h`  | IP/ICMP structs and constants        |
| `slides/`       | Presentation (Pandoc + Reveal.js)    |

## Slides

The `slides/` directory contains a Reveal.js presentation (Pandoc) that walks through ICMP, encapsulation, and this POC.

**Prerequisites:** [Pandoc](https://pandoc.org/) (and network access at build time if you want embedded assets).

```bash
cd slides
make          # build output/index.html
make serve    # serve at http://localhost:8000
```

To export to PDF (requires Node and `npx`):

```bash
cd slides && make pdf
```

PDF is written to `slides/output/slides.pdf`.

## License

Use and modify as you like; no warranty. Raw sockets and tunneling may be restricted by your network or local policy — use responsibly.
