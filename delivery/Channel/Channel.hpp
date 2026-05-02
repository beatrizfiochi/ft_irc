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
        std::string key; // password
        int userLimit;

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

        void setTopic(const std::string& topic);
        const std::string getTopic(void) const;

        const std::set<int>& getMembers() const;
};

#endif // CHANNEL_HPP
