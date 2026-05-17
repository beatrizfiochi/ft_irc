*This project has been created as part of the 42 curriculum by cmoura-p, bfiochi-, djunho.*

# ft_irc

## Description
`ft_irc` is an IRC server written in C++98. It accepts multiple TCP/IP clients simultaneously, keeps all I/O non-blocking, and processes IRC commands through a single `epoll`-based event loop. The server is designed to work with a real IRC client and to rebuild partial incoming packets into complete `CRLF`-terminated messages.

This project does not include an IRC client or server-to-server communication.

The repository also ships the two **bonus** features (built only on demand via `make bonus`):
- **File transfer**: works out of the box with any DCC-capable IRC client; the server relays the CTCP `DCC SEND` payload via PRIVMSG, the file bytes flow client-to-client.
- **A small IRC bot** (`ircbot`): a separate companion program that connects to the server like a regular client and reacts to channel commands.

## Instructions

### Build

From the `delivery/` directory:

```bash
make            # builds ircserv (mandatory)
make bonus      # builds ircserv AND ircbot
make debug      # rebuilds with verbose logs (LOG_DBG enabled) and -ggdb
make re         # full clean rebuild
make fclean     # remove all objects and binaries
```

### Run the server

```bash
./ircserv <port> <password>
```

- `port`: listening TCP port
- `password`: connection password required by clients

### Run the bot (bonus)

After `make bonus`:

```bash
./ircbot <host> <port> <password> <nick> <channel>
```

Example (with the server running on `127.0.0.1:6667` with password `abc`):

```bash
./ircbot 127.0.0.1 6667 abc botty '#lobby'
```

The bot connects, registers (PASS / NICK / USER), joins the given channel, and reacts to channel chatter. Ctrl+C triggers a graceful `QUIT`.

Supported commands (sent as a regular `PRIVMSG` to either the channel or the bot's nick):

| Command         | Reply                                                              |
|-----------------|--------------------------------------------------------------------|
| `!echo <text>`  | repeats `<text>` back                                              |
| `!site`         | replies with the project's repository URL                          |

If the command came from a channel, the bot replies into that channel; if it came as a private message, the bot replies privately to the sender.

### Reference Client
Use a real IRC client to validate the server behavior. A typical choice is `irssi`, but any standard IRC client can be used. The DCC file-transfer bonus has been verified with `hexchat`.

### Quick Test

```bash
nc -C 127.0.0.1 6667
```

Send commands in parts to verify that the server buffers partial input correctly before executing it.

## Project Scope

The mandatory IRC server behavior includes:

- Authentication with `PASS`
- Nickname management with `NICK`
- User registration with `USER`
- Channel handling with `JOIN`
- Private messaging with `PRIVMSG`
- Channel broadcasts to joined members
- Channel operator commands and channel modes (`KICK`, `INVITE`, `TOPIC`, `MODE` with `i`, `t`, `k`, `o`, `l`)
- IRC numeric and error replies

The bonus part adds:
- DCC file transfer (relayed transparently via PRIVMSG)
- An IRC bot (`ircbot`) reacting to `!echo` and `!site`

## Resources
- RFC 1459 — Internet Relay Chat Protocol
- RFC 2812 — Internet Relay Chat: Client Protocol
- `man 2 socket`, `bind`, `listen`, `accept`, `connect`
- `man 2 send`, `recv`
- `man 2 fcntl`
- `man 2 poll`, `man 7 epoll`
- `man 3 getaddrinfo`
- `man 7 signal` (specifically: which syscalls are EINTR-safe vs SA_RESTART)

### AI Usage
AI was used to summarize the subject requirements, draft the README structure, and polish the English wording for the project overview and instructions. The final text was reviewed and adapted manually.

## Notes
- The code targets the C++98 standard (verified clean under `clang++ -std=c++98 -pedantic -Wall -Wextra` across the whole tree).
- The server uses non-blocking file descriptors and a single `epoll` event loop (level-triggered).
- The bot uses a single non-blocking `poll()` on its own connection.
- Executables: `ircserv` (mandatory) and `ircbot` (bonus, only built via `make bonus`).
- Logging: build-time gated via `log.hpp`. Default build emits `INF`/`WRN`/`ERR`; `make debug` additionally enables `DBG`.

