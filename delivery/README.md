*This project has been created as part of the 42 curriculum by cmoura-p, bfiochi-, djunho.*

# ft_irc

## Description
`ft_irc` is an IRC server written in C++98. It accepts multiple TCP/IP clients simultaneously, keeps all I/O non-blocking, and processes IRC commands through a single `epoll`-based event loop. The server is designed to work with a real IRC client and to rebuild partial incoming packets into complete `CRLF`-terminated messages.

This project does not include an IRC client or server-to-server communication.

## Instructions

### Build
From the `delivery/` directory:

```bash
make
```

### Run

```bash
./ircserv <port> <password>
```

- `port`: listening TCP port
- `password`: connection password required by clients

### Reference Client
Use a real IRC client to validate the server behavior. A typical choice is `irssi`, but any standard IRC client can be used.

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
- Channel operator commands and channel modes
- IRC numeric and error replies

## Resources
- RFC 1459 - Internet Relay Chat Protocol
- RFC 2812 - Internet Relay Chat: Client Protocol
- `man 2 socket`
- `man 2 bind`
- `man 2 listen`
- `man 2 accept`
- `man 2 send`
- `man 2 recv`
- `man 7 epoll`
- `man 2 fcntl`

### AI Usage
AI was used to summarize the subject requirements, draft the README structure, and polish the English wording for the project overview and instructions. The final text was reviewed and adapted manually.

## Notes
- The code targets the C++98 standard.
- The server uses non-blocking file descriptors.
- The executable is named `ircserv`.

## TODOs

- [ ] Reminder to add 42Header in all files
- [ ] Audit handlers that call `sendReply` without checking the return value: an inline disconnect on a write failure leaves `client` as a dangling reference for any subsequent statement or loop iteration (and the outer `processBufferedMessages` loop still touches the read buffer). Affected sites: `CommandChannelOps.cpp` `handleInvite` (RPL_INVITING followed by INVITE forward) and `handleTopic` (RPL_TOPIC followed by `return 0`); `CommandJoin.cpp` `handleJoin` per-channel loop (ERR_NOSUCHCHANNEL / ERR_INVITEONLYCHAN / ERR_BADCHANNELKEY / ERR_CHANNELISFULL paths that `continue`); `CommandHandler.cpp` `handlePrivMsg` per-target loop. Fix pattern: capture the return and propagate the negative value, like `handleNick` and `handleJoin`'s broadcast sites already do. (`broadcastMsg`/`broadcastTo` themselves are now safe — they skip the sender inside the loop and only echo to sender as the very last step.)
- [x] Should run as `./ircserv <port> <password>`
- [x] README: Fill the Description section
- [x] README: Fill the Instruction section
- [x] README: Fill the Resources section
- [x] The server must be capable of handling multiple clients simultaneously without hanging.
- [x] Forking is prohibited. All I/O operations must be non-blocking.
- [x] Only 1 poll() (or equivalent) can be used for handling all these operations (read, write, but also listen, and so forth).
- [x] Connect to any client. Use a client as reference
- [x] Communication between client and server has to be done via TCP/IP (v4 or v6)
- Features to implement:
    - [ ] You must be able to authenticate, set a nickname, a username, join a channel, send and receive private messages using your reference client.
        - [x] NICK
        - [x] USER
        - [x] JOIN
        - [x] PRIVMSG
        - [x] PASS
        - [ ] PART
        - [ ] NAMES
        - [x] 421 ERR_UNKNOWNCOMMAND - when we receive an unknown command
        - [ ] check others...
    - [x] All the messages sent from one client to a channel have to be forwarded to every other client that joined the channel.
    - [x] You must have operators and regular users.
    - Then, you have to implement the commands that are specific to channel operators:
        - [ ] KICK - Eject a client from the channel
        - [ ] INVITE - Invite a client to a channel
        - [ ] TOPIC - Change or view the channel topic
        - [ ] MODE - Change the channel’s mode:
            - [ ] i: Set/remove Invite-only channel
            - [ ] t: Set/remove the restrictions of the TOPIC command to channel operators
            - [ ] k: Set/remove the channel key (password)
            - [ ] o: Give/take channel operator privilege
            - [ ] l: Set/remove the user limit to channel
