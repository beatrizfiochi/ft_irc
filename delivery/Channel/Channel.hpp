#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <set>

class Client;

class Channel {
    private:
        std::string name;
        std::string topic;

        std::set<int> members;
        std::set<int> operators;

        bool inviteOnly;
        bool topicRestricted;
        // password
        std::string key;
        // Maxmimum number of members in the channel. -1 if disabled
        int userLimit;
        // modes
        std::string modeList;
        // invite
        std::set<int> invitedClients;

    public:
        Channel(void); // default constructor
        Channel(const std::string &channelName);
        Channel(const Channel& other); // copy constructor
        Channel& operator=(const Channel& other); // copy assign operator
        ~Channel(void); // destructor

        const std::string getName(void) const;

        void addClient(int fd);
        void removeClient(int fd);

        bool isMember(int fd) const;
        bool isOperator(int fd) const;
        bool isTopicRestricted() const;

        void setTopic(const std::string& topic);
        const std::string getTopic(void) const;

        const std::set<int>& getMembers() const;

        bool isInviteOnly(void) const;
        void setInviteOnly(bool flag);

        void inviteClient(int fd);
        void removeInvite(int fd);
        bool isClientInvited(int fd);

        void setKey(const std::string &key);
        bool checkKey(const std::string &key);
        bool hasKey(void) const;

        bool isFull(void) const;

        void enableInviteOnly();
        void disableInviteOnly();
        void enableTopicRestricted();
        void disableTopicRestricted();
        void addOperator(int fd);
        void removeOperator(int fd);
        void setPass(const std::string &key);
        void removePass();
        void setLimit(long value);
        void removeLimit();
        const std::string getModeList(void) const;
    };

#endif // CHANNEL_HPP
