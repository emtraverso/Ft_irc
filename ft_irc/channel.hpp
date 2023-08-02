#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <list>
#include "client.hpp"

class Channel
{
    public:
        Channel();
        ~Channel();
        void setName(std::string name);
        void setPassword(std::string password);
        void addSockClient(int client);
        void removeSockClient(int client);
        void setTopic(std::string topic);
        void setModes(std::string modes);
        void setkick(int kick);
        void setInvites(int invites);
        void setAdmin(int admin);
        void setInviteEnable(int i);
        int getInviteEnable();
        void removeAdmin(int admin);
        void SetPassFlag(int i);
        void setMaxClient(int i);
        void setTopicEnable(int i);
        int isClientInChannel(int socket);
        int isClientAdmin(int socket);
        int isPassSet();
        int getMaxClient();
        int getNClient();
        int addClientToLimit();
        int removeClientFromLimit();
        int resetClientLimit();
        int isTopicEnable();
        int isClientInvited(int ClientToFind);
        void removeInvited(int ClientToFind);
        void setClientToLimit();

        std::string getName();
        std::string getPassword();
        std::string getTopic();
        std::list<int> &getSockClients();
        std::list<std::string> &getModes();
        std::list<int> &getkick();
        std::list<int> &getInvites();
        std::list<int> &getAdmin();
        void clearAll();

    private:
        std::string _name;
        std::string _password;
        std::string _topic;
        int _MaxClientLimit;
        int _currentNClient;
        int _isPassSet;
        int _inviteEnable;
        int _topicEnable;
        std::list<std::string> _modes;
        std::list<int> _kick;
        std::list<int> _invites;
        std::list<int> _admin;
        std::list<int> _SockClients;
};