#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <map>

class Client;

class Channel {
    private:
        std::string name;
        std::string topic;

        std::map<int, Client*> members; // fd -> client
        std::map<int, Client*> operators;

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

        void addClient(Client& client);
        void removeClient(Client& client);

        bool isMember(Client& client) const;
        bool isOperator(Client& client) const;

        void broadcast(const std::string& msg, Client *exclude = NULL);

        void setTopic(const std::string& topic);
        const std::string getTopic(void) const;
};

#endif // CHANNEL_HPP
