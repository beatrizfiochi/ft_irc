#ifndef _IRC_PARSE_HPP_
#define _IRC_PARSE_HPP_

#include <string>
#include <vector>

// Parsed IRC line. Grammar (RFC 1459/2812):
//   <message> ::= [':' <prefix> ' '] <command> { ' ' <param> } [ ' :' <trailing> ]
// `prefix` is empty when no leading ':' prefix was present.
// `params` includes the trailing as its last element when hasTrailing() is true.
class IrcMessage {
public:
    IrcMessage(void);
    IrcMessage(const IrcMessage &other);
    IrcMessage &operator=(const IrcMessage &other);
    ~IrcMessage(void);

    const std::string &getPrefix(void) const;
    const std::string &getCommand(void) const;
    const std::vector<std::string> &getParams(void) const;
    bool hasTrailing(void) const;

    void setPrefix(const std::string &prefix);
    void setCommand(const std::string &command);
    void addParam(const std::string &param);
    void setTrailing(bool trailing);

private:
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    bool trailing;
};

// Tokenize a single IRC line (no CRLF; caller strips). Returns false only on
// gross structural failure (no command found). Does not validate command name
// or parameter content — that is the caller's job.
bool tokenizeIrcLine(const std::string &line, IrcMessage &out);

// Result of tokenizeNextMessage(). OK means out is populated and the frame
// was consumed from the buffer; the *_DROPPED results also consume the frame
// (so the caller advances past the bad input). NO_FRAME leaves the buffer
// untouched.
enum TokenizeResult {
    TOKENIZE_OK,
    TOKENIZE_NO_FRAME,
    TOKENIZE_TOO_LONG,
    TOKENIZE_EMPTY,
    TOKENIZE_MALFORMED
};

// Pop the next \r\n-terminated frame from buf and tokenize it.
// Enforces the RFC 1459 §2.3 size cap (512 bytes including CRLF) internally.
// On TOKENIZE_OK, out is populated and the frame (plus its CRLF) is removed
// from buf. On TOKENIZE_TOO_LONG / TOKENIZE_EMPTY / TOKENIZE_MALFORMED the
// frame is still consumed; out is reset to default.
TokenizeResult tokenizeNextMessage(std::string &buf, IrcMessage &out);

// Extract the nick portion from an IRC prefix of the form "nick!user@host".
// If no '!' is present the whole prefix is returned.
std::string nickFromPrefix(const std::string &prefix);

#endif // _IRC_PARSE_HPP_
