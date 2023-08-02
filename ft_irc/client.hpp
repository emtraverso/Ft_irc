#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <list>

class Client
{
    public:
        Client();
        ~Client();
        void setNick(std::string nick);
        void setClientname(std::string Clientname);
        void setSocket(int socket);
        void addMessage();
        void setMessage(int i);
        int getNMessage();
        std::string &getBuffer();
        void addBuffer(std::string buffer);
        void setBuffer(std::string buffer);
        std::string &getNick();
        std::string &getClientname();
        int &getSocket();
    private:
        std::string _nick;
        std::string _clientname;
        std::string _buffer;
        int _socket;
        int _message;
};