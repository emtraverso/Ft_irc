#include "client.hpp"
#include "server.hpp"
#include "channel.hpp"


Channel::Channel()
{}

Channel::~Channel()
{
    
}
void Channel::setName(std::string name)
{
    _name = name;
}
void Channel::setPassword(std::string password)
{
    _password = password;
}
int Channel::isPassSet()
{
    return _isPassSet;
}
void Channel::SetPassFlag(int i)
{
    _isPassSet = i;
}

void Channel::setMaxClient(int i)
{
    _MaxClientLimit = i;
}
int Channel::getNClient()
{
    return _currentNClient;
}
int Channel::addClientToLimit()
{
    if (_currentNClient < _MaxClientLimit)
    {
        _currentNClient++;
        return (1);
    }
    return (0);
}
void Channel::setClientToLimit()
{
        _currentNClient = 1;
}

int Channel::removeClientFromLimit()
{
    if (_currentNClient > 0)
    {
        _currentNClient--;
        return (1);
    }
    return (0);
}

int Channel::resetClientLimit()
{
    _currentNClient = 0;
    return (1);
}

int Channel::getMaxClient()
{
    return _MaxClientLimit;
}

void Channel::addSockClient(int client)
{
    _SockClients.push_back(client);
    _currentNClient = _SockClients.size();

}
void Channel::removeSockClient(int client)
{
    _SockClients.remove(client);
    _currentNClient = _SockClients.size();
}

void Channel::setTopic(std::string topic)
{
    _topic = topic;
}

void Channel::setModes(std::string modes)
{
    _modes.push_back(modes);
}

void Channel::setkick(int kick)
{
    _kick.push_back(kick);
    removeClientFromLimit();
}

void Channel::setInviteEnable(int i)
{
    _inviteEnable = i;
}
int Channel::getInviteEnable()
{
    return _inviteEnable;
}

int Channel::isClientInvited(int ClientToFind)
{
    if (_invites.size() == 0)
        return (0);
    std::list<int>::iterator it;
    for (it = _invites.begin(); it != _invites.end(); it++)
    {
        if (*it == ClientToFind)
            return (1);
    }
    return (0);
}

void Channel::removeInvited(int ClientToFind)
{
    if (_invites.size() == 0)
        return ;
    std::list<int>::iterator it;
    for (it = _invites.begin(); it != _invites.end(); it++)
    {
        if (*it == ClientToFind)
            break;
    }
    if (it != _invites.end())
        _invites.remove(ClientToFind);    
}

int Channel::isTopicEnable()
{
    return _topicEnable;
}

void  Channel::setTopicEnable(int i)
{
    _topicEnable = i;
}

void Channel::setInvites(int invites)
{
    _invites.push_back(invites);
}

void Channel::setAdmin(int admin)
{
    _admin.push_back(admin);
}

std::string Channel::getName()
{
    return _name;
}
std::string Channel::getPassword()
{
    return _password;
}
std::list<int> &Channel::getSockClients()
{
    return _SockClients;
}

std::string Channel::getTopic()
{
    return _topic;
}

std::list<std::string>  &Channel::getModes()
{
    return _modes;
}

std::list<int> &Channel::getkick()
{
    return _kick;
}

std::list<int> &Channel::getInvites()
{
    return _invites;
}

std::list<int> &Channel::getAdmin()
{
    return _admin;
}

void Channel::removeAdmin(int admin)
{
    _admin.remove(admin);
}

int Channel::isClientInChannel(int socket)
{
    if (_SockClients.size() == 0)
        return (0);
    std::list<int>::iterator it;
    for (it = _SockClients.begin(); it != _SockClients.end(); it++)
    {
        if (*it == socket)
            return (1);
    }
    return (0);
}

int Channel::isClientAdmin(int socket)
{
    if (_admin.size() == 0)
        return (0);
    std::list<int>::iterator it;
    for (it = _admin.begin(); it != _admin.end(); it++)
    {
        if (*it == socket)
            return (1);
    }
    return (0);
}

void Channel::clearAll()
{
    _name.clear();
    _password.clear();
    _topic.clear();
    _modes.clear();
    _kick.clear();
    _invites.clear();
    _admin.clear();
    _SockClients.clear();
}