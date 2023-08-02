#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sstream>
#include <csignal>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <cerrno>


#include "channel.hpp"
#include "client.hpp"

class Channel;
class Client;

class Server
{
public:
    Server(std::string pass, int port);
    ~Server();
    std::map<int, Client> &getClients();
    std::map<std::string, Channel> &getChannels();
    std::string &getPass();
    std::string &getNameserver();
    int getPort();
    int getServerSock();
    int getFds(int i);
    void setPass(std::string pass);
    void setNameserver(std::string nameserver);
    void addClient(int usr, Client Client);
    void addChannel(std::string name, Channel &channel);
    void ServerRun();
    void setPort(int port);
    void setServerSock(int socket);
    int CreateSocketConnection();
    int indexClient();
    void initClient();
    int getMaxClients();
    int getClientSocket(std::string nick);
    void messageProcessing(const std::string &command, int j);
    void goToNick(std::string argsCmd, int j);
    void goToPass(std::string argsCmd, int j);
    void goToUser(std::string argsCmd, int j);
    void goToJoin(std::string argsCmd, int j);
    void goToMode(std::string argsCmd, int j);
    void goToLeave(std::string argsCmd, int j);
    void goToWho(std::string argsCmd, int j);
    void goToMsg(std::string argsCmd, int j);
    void goToQuit(std::string argsCmd, int j);
    void goToKick(std::string argsCmd, int j);
    void goToInvite(std::string argsCmd, int j);
    void goToTopic(std::string argsCmd, int j);
    void goToBot(std::string argsCmd, int j);
    void findEraseSpaces(std::string &str);
    void updateAdminList(std::string message, std::string ch);
    void sendMsgToCh(std::string message, std::string ch, int j);
    void LeaveClientFormAllChannels(int socketClient);
    void clearAll();
    void clearPartialCommand();
private:
    Server();
    std::map<int, Client> _client;
    std::map<std::string, Channel> _channels;
    std::map<int, std::string> _partialCommands;
    std::string _command;
    std::string _pass;
    int _port;
    int _serverSock;
    std::string _nameserver;
    int _maxClient;
    struct pollfd *_fds;
    int _initPass;
};

int ft_stoi(const std::string &str);