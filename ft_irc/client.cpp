#include "client.hpp"
#include "server.hpp"
#include "channel.hpp"


Client::Client(){}

Client::~Client(){}

void Client::setNick(std::string nick) 
{_nick = nick;}

void Client::setClientname(std::string clientname) 
{_clientname = clientname;}

void Client::setSocket(int socket) 
{_socket = socket;}

std::string &Client::getNick() 
{return _nick;}

std::string &Client::getClientname() 
{return _clientname;}

int &Client::getSocket() 
{return _socket;}

void Client::addMessage()
{
    _message++;
}
void Client::setMessage(int i)
{
    _message = i;
}
int Client::getNMessage()
{
    return _message;
}

std::string &Client::getBuffer()
{
    return _buffer;
}

void Client::addBuffer(std::string buffer)
{
    _buffer += buffer;
}

void Client::setBuffer(std::string buffer)
{
    _buffer = buffer;
}