*This project has been created as part of the 42 curriculum by cmoura-p, bfiochi-, djunho*

# ft_irc

# Description

...

This project does not developed:
- IRC client
- server-to-server communication

# Introduction

Internet Relay Chat or IRC is a text-based communication protocol on the Internet.
It offers real-time messaging that can be either public or private. Users can exchange
direct messages and join group channels.
IRC clients connect to IRC servers in order to join channels. IRC servers are connected
together to form a network.

# Instructions

## Build

## Running

## Testing

```
nc -C 127.0.0.1 6667
# Use ctrl+D to send the command in several parts: ’com’, then ’man’, then ’d\n’.
```

# Resources

# TODOs

- [ ] Should run as `./ircserv <port> <password>`
- [ ] README: Fill the Description section
- [ ] README: Fill the Instruction section
- [ ] README: Fill the Resources section
- [ ] The server must be capable of handling multiple clients simultaneously without hanging.
- [ ] Forking is prohibited. All I/O operations must be non-blocking.
- [ ] Only 1 poll() (or equivalent) can be used for handling all these operations (read, write, but also listen, and so forth).
- [ ] Connect to any client. Use a client as reference
- [ ] Communication between client and server has to be done via TCP/IP (v4 or v6)
- Features to implement:
    - [ ] You must be able to authenticate, set a nickname, a username, join a channel, send and receive private messages using your reference client.
    - [ ] All the messages sent from one client to a channel have to be forwarded to every other client that joined the channel.
    - [ ] You must have operators and regular users.
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

