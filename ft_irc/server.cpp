#include "server.hpp"
#include "channel.hpp"
#include "client.hpp"

#include <climits>

int ft_stoi(const std::string& str)
{
    const char* cstr = str.c_str();
    char* end;
    int base = 10;

    // Ignora gli spazi iniziali
    while (std::isspace(*cstr))
        ++cstr;

    // Controlla il segno
    int sign = 1;
    if (*cstr == '+' || *cstr == '-')
    {
        if (*cstr == '-')
            sign = -1;
        ++cstr;
    }

    // Determina la base
    if (*cstr == '0')
    {
        if (cstr[1] == 'x' || cstr[1] == 'X')
        {
            base = 16;
            cstr += 2;
        }
        else
        {
            base = 8;
            ++cstr;
        }
    }

    // Conversione effettiva
    long result = std::strtol(cstr, &end, base);
    if (cstr == end)
        throw std::invalid_argument("INVALID ARGUMENT");

    // Controllo overflow
    if (result > INT_MAX || result < INT_MIN)
        throw std::out_of_range("NUMERONE GROSSO");

    return static_cast<int>(result) * sign;
}


Server::Server()
{}

Server::Server(std::string pass, int port)
{
    _pass = pass;
    _port = port;
    _nameserver = "MicheleIRC";
    _maxClient = 50;
    _initPass = 0;
    // inizializza _partialCommands con una chiave/valore vuoto
    //_partialCommands.insert(std::make_pair(0, ""));
    _fds = new struct pollfd[_maxClient];
}

Server::~Server()
{
    delete[] _fds;
    //clearAll();
}

std::map<int, Client> &Server::getClients() 
{return _client;}

std::map<std::string, Channel> &Server::getChannels() 
{return _channels;}

std::string &Server::getPass() 
{return _pass;}

std::string &Server::getNameserver() 
{return _nameserver;}

int Server::getPort() 
{return _port;}

void Server::setPass(std::string pass) 
{_pass = pass;}

void Server::setNameserver(std::string nameserver) 
{_nameserver = nameserver;}

void Server::initClient()
{
    for (int i = 1; i < _maxClient; i++)
    {
        _fds[i].fd = 0;
        _fds[i].events = POLLIN;
        _fds[i].revents = 0;
    }
}

int Server::indexClient()
{
    int i = 1;
    for (; i < _maxClient; i++)
    {
        if (_fds[i].fd == -1)
            return i;
    }

    for (i = 1; i < _maxClient; i++)
    {
        if (_fds[i].fd == 0)
            break;
    }
    return i;
}

int Server::getMaxClients()
{return _maxClient;}

void Server::setPort(int port) 
{_port = port;}

void Server::addClient(int usr, Client Client)
{
    _client.insert(std::make_pair(usr, Client));
}
void Server::addChannel(std::string name, Channel &channel)
{
    _channels.insert(std::make_pair(name, channel));
}

int Server::getServerSock()
{return _serverSock;}

void Server::setServerSock(int socket)
{_serverSock = socket;}

int Server::getFds(int i)
{
    return _fds[i].fd;
}

int Server::CreateSocketConnection()
{
    // Creazione del socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // da quello che ho capito AF_INET = IPv4, SOCK_STREAM = TCP, 0 = IP
    if (sockfd < 0) {
        std::cerr << "Errore creazione del socket." << std::endl;
        return -1;
    }
    setServerSock(sockfd); // salvo il socket del server
    // Creazione dell'indirizzo del server
    sockaddr_in serverAddress; // struttura che contiene l'indirizzo del server
    serverAddress.sin_family = AF_INET; // sin_family e' una variabile della struttura in cui bisogna specificare che tipo di ip stiamo utilizzando, nel nostro caso ipv4
    serverAddress.sin_port = htons(_port); // imposto la porta del server nella struttura, htons e' una funzione che converte il numero di porta da formato host a formato network
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    // Connessione al server
    if (bind(sockfd, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("Errore nella bind");
        std::cerr << "Connessione al server fallita." << std::endl;
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void Server::ServerRun()
{
    initClient();
    std::cout << "Server is running" << std::endl;
    int flag = fcntl(_serverSock, F_GETFL, 0);
   
    fcntl(_serverSock, F_SETFL, flag | O_NONBLOCK);
    
    _fds[0].fd = _serverSock;
    _fds[0].events = POLLIN;
    _fds[0].revents = 0;
    int response = 0;
    int i = 1;
   
    while(1)
    {
        response = poll(_fds, _maxClient, -1); 
        if(response == -1)
        {
            std::cerr << "Errore nella poll" << std::endl;
            return;
        }
        if (_fds[0].revents && POLLIN)
        {
            int clientSocket = accept(_serverSock, NULL, NULL);
            if(clientSocket < 0)
            {
                std::cerr << "Errore nell'accept" << std::endl;
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                    continue;
                else
                    return;
                return;
            }
            std::cout << "Client socket is:" << clientSocket << std::endl;
            i = indexClient();
            _fds[i].fd = clientSocket;
            _fds[i].events = POLLIN;
            _fds[i].revents = 0;
            _client.insert(std::make_pair(clientSocket, Client()));
            _client[clientSocket].setSocket(clientSocket);
            _client[clientSocket].setMessage(0);
        }
        for(long unsigned int j = 1; j <= _client.size(); j++)
        {
            if (_fds[j].fd == -1)
                continue;
            if (_fds[j].revents & POLLIN)
            {
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                response = recv(_fds[j].fd, buffer, sizeof(buffer), 0);
                if(response == -1)
                {
                    std::cerr << "Socket fd: " << _fds[j].fd  << " J is " << j << std::endl;
                    std::cerr << "Errore nella recv: " << response << std::endl;
                    if(errno == EAGAIN || errno == EWOULDBLOCK)
                        continue;
                    else
                        return;
                    continue;
                }
                else if(response == 0)
                {
                    std::cout << "Client " << _fds[j].fd << " e' disconnesso" << std::endl;
                    _partialCommands.erase(_fds[j].fd);
                    _fds[j].fd = -1;
                    _client.erase(_fds[j].fd);
                    close(_fds[j].fd);
                    continue;
                }
                else
                {
                    
                    std::string receivedData(buffer, response);
                    std::string& partialCommand = _partialCommands[_fds[j].fd];
                    partialCommand += receivedData;

                    if (!partialCommand.empty())
                    {
                        // Verifica se il comando completo è stato ricevuto (termina con "\n")
                        size_t newlinePos = partialCommand.find('\n');
                        while (newlinePos != std::string::npos)
                        {
                            _command = partialCommand.substr(0, newlinePos); // Comando completo
                            partialCommand = partialCommand.substr(newlinePos + 1); // Parte rimanente del comando
                            _client[_fds[j].fd].addMessage();
                            messageProcessing(_command.c_str(), j); // Elabora il comando completo
                            newlinePos = partialCommand.find('\n');
                        }
                    }
                }
            }
        }
    }
}

void Server::clearPartialCommand()
{
    _partialCommands.clear();
}
void Server::messageProcessing(const std::string& command, int j)
{
    std::string commandSet = ""; // initialize commandSet to an empty string
    std::string argsCmd; // initialize argsCmd to an empty string
    std::istringstream iss(command);
    if (!(iss >> commandSet))
    {
        commandSet = "";
    }
    while (iss)
    {
        std::string tmp;
        iss >> tmp;
        argsCmd += tmp + " ";
    }
    argsCmd = argsCmd.substr(0, argsCmd.size() - 1);
    std::cout << "Command: " << command << std::endl;
    std::cout << "CommandSet: " << commandSet << std::endl;
    std::cout << "Args: " << argsCmd << std::endl;
    if (_fds[j].fd != -1 && _client.find(_fds[j].fd) != _client.end())
    {
        std::map<int, Client>::iterator clientIter = _client.find(_fds[j].fd);
        if (clientIter != _client.end())
        {
            if (clientIter->second.getNMessage() == 2 && commandSet != "PASS")
            {
                std::string message = "Password non inserita!";
                send(_fds[j].fd, message.c_str(), message.size(), 0);
                close(_fds[j].fd);
                _fds[j].fd = -1;
                return;
            }

            if (commandSet == "CAP" || commandSet == "CAPLS")
            {
                _initPass = 1;
                std::cout << "Comando CAPLS riconosciuto" << std::endl;
            }
            if (commandSet == "NICK")
                goToNick(argsCmd, j);
            else if (commandSet == "PASS")
            {
                if (_initPass == 1)
                {
                    goToPass(argsCmd, j);
                    std::cout << "Comando PASS riconosciuto" << std::endl;
                }
                else
                    std::cout << "Comando PASS NON valido..." << std::endl;
                _initPass = 0;
            }
            else if (commandSet == "USER")
                goToUser(argsCmd, j);
            else if (commandSet == "JOIN")
                goToJoin(argsCmd, j);
            else if (commandSet == "MODE")
                goToMode(argsCmd, j);
            else if (commandSet == "PART" || commandSet == "LEAVE")
                goToLeave(argsCmd, j);
            else if (commandSet == "WHO")
                goToWho(argsCmd, j);
            else if (commandSet == "PRIVMSG")
                goToMsg(argsCmd, j);
            else if (commandSet == "QUIT")
                goToQuit(argsCmd, j);
            else if (commandSet == "KICK")
                goToKick(argsCmd, j);
            else if (commandSet == "INVITE")
                goToInvite(argsCmd, j);
            else if (commandSet == "TOPIC")
                goToTopic(argsCmd, j);
            else if (commandSet == "PING")
                send(_fds[j].fd, "PONG\r\n", 6, 0);
            else if (commandSet == "BOT")
                goToBot(argsCmd, j);
            else
            {
                std::cout << "Comando " << commandSet << " non riconosciuto o errore" << std::endl;
                return ;
            }
        }
        return ;
    }
    return ;
}

void Server::goToBot(std::string argsCmd, int j)
{
    std::string message = argsCmd;
    std::istringstream iss(message);
    std::string arg;

    iss >> arg;

    findEraseSpaces(arg);
    std::string messageadv = _nameserver + "!*!*!*!*! LISTA UTENTE SERVER IRC !*!*!*!*!" +"\r\n";
    send(_fds[j].fd, messageadv.c_str(), messageadv.size(), 0);
    
    if (arg == "usr")
    {
        std::cout << "ENTRO NEL BOT" << std::endl;
        for(std::map<int, Client>::iterator it = _client.begin(); it != _client.end(); ++it)
        {
            std::cout << "Client: " << it->first << " Nick: " << it->second.getNick() << std::endl;
            std::string message = _nameserver + " Nick: " + it->second.getNick() + "\r\n";
            send(_fds[j].fd, message.c_str(), message.size(), 0);
        }
    }
}


void Server::goToNick(std::string argsCmd, int j)
{
    std::string nick = argsCmd;
    if (nick.length() > 9)
    {
        std::cout << "Il nick deve essere lungo al massimo 9 caratteri" << std::endl;
        close(_fds[j].fd);
        _client.erase(_fds[j].fd);
        _fds[j].fd = -1;
    }
    else
    {
        findEraseSpaces(nick);
        _client[_fds[j].fd].setNick(nick);
        std::cout << "Il nick del client " << _fds[j].fd << " e' " << _client[_fds[j].fd].getNick() << std::endl;
    }
}

void Server::goToPass(std::string argsCmd, int j)
{
    std::string pass = argsCmd;
    size_t pos = pass.find(':');
    if (pos != std::string::npos)
        pass.erase(pos, 1);
    findEraseSpaces(pass);
    std::string correctPass = getPass();
    findEraseSpaces(correctPass);
    
    if (pass != correctPass)
    {
        std::cout << "Password errata" << std::endl;
        close(_fds[j].fd);
        _client.erase(_fds[j].fd);
        _fds[j].fd = -1;
    }
    else
        std::cout << "Password corretta complimenti sei dentro" << std::endl;
}

void Server::findEraseSpaces(std::string &str)
{
    str.erase(0, str.find_first_not_of(" \t\n\r\f\v"));
    str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
}

void Server::goToUser(std::string argsCmd, int j)
{
    size_t pos = argsCmd.find(':');
    std::string user = argsCmd.substr(pos + 1);
    findEraseSpaces(user);
    _client[_fds[j].fd].setClientname(user);
    std::cout << "L'username del client " << _fds[j].fd << " e' " << _client[_fds[j].fd].getClientname() << std::endl;
    std::string message = "001 " + _client[_fds[j].fd].getNick() + " MicheleIRC :Welcome to the server MicheleIRC " + _client[_fds[j].fd].getNick() + "\r\n";
    send(_fds[j].fd, message.c_str(), message.size(), 0);
}

void Server::goToJoin(std::string argsCmd, int j)
{
    std::string message = argsCmd;
    std::istringstream iss(message);
    std::string join;
    std::string pass;

    iss >> join;
    iss >> pass;

    if (join == "" || join == "#" || join.find("#") == std::string::npos)
    {
        std::string error = _nameserver + " :formattazione errata!\r\n";
        send(_fds[j].fd, error.c_str(), error.size(), 0);
        return ;
    }
    
    findEraseSpaces(join);
    findEraseSpaces(pass);

    int ClientToFind = _fds[j].fd;
    int flg = 0;

    std::map<std::string, Channel>::iterator itf = getChannels().begin();
    for (; itf != getChannels().end(); ++itf)
    {
        if (itf->first == join)
        {
            if (itf->second.getMaxClient() && itf->second.getNClient() >= itf->second.getMaxClient())
            {
                std::string err =  _nameserver + " Impossibile entrare nel canale, limite di utenti raggiunto!\r\n";
                send(_fds[j].fd, err.c_str(), err.size(), 0);
                return ;
            }
            flg = 1;
            if(!itf->second.isClientInChannel(ClientToFind))
            {
                if(_channels[join].getInviteEnable() && !itf->second.isClientInvited(ClientToFind))
                {
                    std::string error;
                    error = ":" + _nameserver + " 482 " + _client[_fds[j].fd].getNick() + " " + join + " :Inviti Disabilitati\r\n";
                    send(_fds[j].fd, error.c_str(), error.size(), 0);
                    return ;
                }
                if(itf->second.isPassSet())
                {
                    if(pass == itf->second.getPassword() || itf->second.isClientInvited(ClientToFind))
                    {
                        itf->second.addSockClient(_fds[j].fd);
                        itf->second.removeInvited(ClientToFind);
                    }
                    else
                    {
                        std::string err = _nameserver + " Password errata !!!\r\n";
                        send(_fds[j].fd, err.c_str(), err.size(), 0);
                        return ;
                    }
                }
                else
                {
                    itf->second.addSockClient(_fds[j].fd);
                    itf->second.removeInvited(ClientToFind);
                }
            }
            break;
        }
    }
    if(!flg)
    {
        Channel channel;
        addChannel(join, channel);
        if(!_channels[join].isClientInChannel(ClientToFind))
            _channels[join].addSockClient(_fds[j].fd);
        _channels[join].SetPassFlag(false);
        _channels[join].setMaxClient(0);
        _channels[join].setClientToLimit();
        _channels[join].setInviteEnable(false);
        _channels[join].setTopic("Welcome to the Jungle!");
    }

    std::string join_message = ":" + _client[_fds[j].fd].getNick() + "! JOIN " + join + "\r\n";
    std::string topicMessage = ":" + _nameserver + " 332 " + _client[_fds[j].fd].getNick() + " " + join + " :" + _channels[join].getTopic() + "\r\n";

    std::list<int>::iterator it, tmpit;
    std::list<int>::iterator itr;
    std::list<int>::iterator ite;
    int size;

    it = _channels[join].getSockClients().begin();
    tmpit = _channels[join].getSockClients().begin();
    ite = _channels[join].getSockClients().end();
    size = _channels[join].getSockClients().size();

    std::string nickList;

    for (; it != ite; ++it)
    {
        if (size == 1)
        {
            if(!_channels[join].isClientAdmin(*it))
            {
                _channels[join].setAdmin(*it);
                nickList += "@" + _client[*it].getNick() + " ";
                std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + join + " +o " + _client[*it].getNick() + "\r\n";
                updateAdminList(message, join);
            }
            break;
        }
        else if (_channels[join].isClientAdmin(*it))
            nickList += "@" + _client[*it].getNick() + " ";
        else
            nickList += _client[*it].getNick() + " ";
    }

    it = tmpit;
    std::string namreply;
    namreply = ":" + _nameserver + " 353 " + _client[_fds[j].fd].getNick() + " = " + join + " :" + nickList + "\r\n";
    std::cout << "***** LISTA UTENTI: " << namreply << std::endl;
    std::string endOfNamesMessage = ":" + _nameserver + " 366 " + _client[_fds[j].fd].getNick() + " " + join + " :End of /NAMES list\r\n";

    for (; it != ite; ++it)
        send(*it, join_message.c_str(), join_message.size(), 0);

    send(_fds[j].fd, topicMessage.c_str(), topicMessage.size(), 0);

    send(_fds[j].fd, namreply.c_str(), namreply.size(), 0);

    send(_fds[j].fd, endOfNamesMessage.c_str(), endOfNamesMessage.size(), 0);
}

void Server::goToMode(std::string argsCmd, int j)
{
    std::string message = argsCmd;
    std::istringstream iss(message);
    std::string ch;
    std::string mode;
    std::string arg;
    iss >> ch;
    iss >> mode;
    iss >> arg;

    findEraseSpaces(ch);
    findEraseSpaces(mode);
    findEraseSpaces(arg);
    int ClientToFind = _fds[j].fd;
    std::map<int, Client>::iterator it = _client.begin();
    std::map<int, Client>::iterator ite = _client.end();
    
    for (; it != ite; ++it)
    {
        if((_channels[ch].isClientAdmin(ClientToFind)) || mode == "+b" || mode == "-b" || arg == "")
        {
            std::cout << "*** BREAK *** " << std::endl;
            break;
        }
    }
    if(!(it != ite))
    {
        std::string error;
        if(!_channels[ch].isClientAdmin(ClientToFind))
            error = ":" + _nameserver + " 482 " + _client[_fds[j].fd].getNick() + " " + ch + " :You're not channel operator\r\n";
        else
            error = ":" + _nameserver + " 482 " + _client[_fds[j].fd].getNick() + " " + ch + " :User not in channel\r\n";
        send(_fds[j].fd, error.c_str(), error.size(), 0);
        std::cout << "Errore: " << error << std::endl;
        return ;
    }

    if((mode == "+o" || mode == "-o") && arg != "")
    {
        if(mode == "+o" && _channels[ch].isClientInChannel(getClientSocket(arg)))
        {
            _channels[ch].setAdmin(it->first);
            std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " +o " + arg + "\r\n";
            updateAdminList(message, ch);
        }
        else if(mode == "-o" && _channels[ch].isClientInChannel(getClientSocket(arg)))
        {
            _channels[ch].removeAdmin(it->first);
            std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " -o " + arg + "\r\n";
            updateAdminList(message, ch);
        }
    }
    else if (mode == "+l" || mode == "-l")
    {
        if (mode == "+l" && arg != "")
        {
            try
            {
                int limit = ft_stoi(arg);
                (void)limit;
            }
            catch (std::out_of_range &e)
            {
                std::string errmsg = _nameserver + " :Devi inserire un Limite valido!\r\n";
                send(_fds[j].fd, errmsg.c_str(), errmsg.size(), 0);
                std::cout << "STOI NON VALIDO" << std::endl;
                return ;
            }
            catch (std::invalid_argument &e)
            {
                std::string errmsg = _nameserver + " :Argomento non valido!\r\n";
                send(_fds[j].fd, errmsg.c_str(), errmsg.size(), 0);
                std::cout << "STOI NON VALIDO" << std::endl;
                return ;
            }
            std::cout << "GLI UTENTI MASSIMI SONO: " << ft_stoi(arg) << " N CLIENT: " << _channels[ch].getSockClients().size() << std::endl;
            if ((long unsigned int)ft_stoi(arg) < _channels[ch].getSockClients().size())
                {
                    std::cout << "MODE +l, ENTRO NEL PRIMO IF IN CUI FACCIO LO STOI DI ARG" << std::endl;
                    std::string error = _nameserver + " Utenti attivi nel canale superiori al limite impostato, Kickare manualmente gli utenti fino al raggiungimento del limite impostato o aspettare che escano\r\n";
                    send(_fds[j].fd, error.c_str(), error.size(), 0);
                    return ;
                }
            else
                {
                    std::cout << "MODE +l, ENTRO NELL'ELSE IN CUI IMPOSTO IL LIMITE DI UTENTI MASSIMI" << std::endl;
                    std::string rispostina = _nameserver + " :Limite impostato correttamente!\r\n";
                    send(_fds[j].fd, rispostina.c_str(), rispostina.size(), 0);
                    _channels[ch].setMaxClient(ft_stoi(arg));
                    std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " +l " + arg + "\r\n";
                }
        }
        else if (mode == "+l" && arg == "")
        {
            std::string errmsg = _nameserver + " :Devi inserire un Limite!\r\n";
            send(_fds[j].fd, errmsg.c_str(), errmsg.size(), 0);
            return ;
        }
        else if (mode == "-l")
        {
            std::cout << "MODE -l, ENTRO NELL'ELSE IF IN CUI DISABILITO IL LIMITE DI UTENTI MASSIMI" << std::endl;
            _channels[ch].setMaxClient(0);
            std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " -l " + arg + "\r\n";
        }
        std::cout << "INVIO IL MESSAGGIO +l O -l" << std::endl;
        send(_fds[j].fd, message.c_str(), message.size(), 0);
    }
    else if (mode == "+t" || mode == "-t")
    {
        if (mode == "+t")
        {
            std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " +t " + arg + "\r\n";
            _channels[ch].setTopicEnable(1);
            send(_fds[j].fd, message.c_str(), message.size(), 0);
        }
        else
        {
            std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " -t " + arg + "\r\n";
            _channels[ch].setTopicEnable(0);
            send(_fds[j].fd, message.c_str(), message.size(), 0);
        }
        
    }
    else if (mode == "+i" || mode == "-i")
    {
        if (mode == "+i")
        {
            _channels[ch].setInviteEnable(1);
            std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " +i " + arg + "\r\n";
        }
        else
        {
            _channels[ch].setInviteEnable(0);
            std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " -i " + arg + "\r\n";
        }
        send(_fds[j].fd, message.c_str(), message.size(), 0);
    }
    else if(mode == "+k" || mode == "-k")
    {
        if (mode == "+k" && arg != "")
        {
            _channels[ch].setPassword(arg);
            _channels[ch].SetPassFlag(1);
            std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " +k " + arg + "\r\n";
            send(_fds[j].fd, message.c_str(), message.size(), 0);
        }
        else if (mode == "+k" && arg == "")
        {
            std::string errmsg = _nameserver + " :Devi inserire una password!\r\n";
            send(_fds[j].fd, errmsg.c_str(), errmsg.size(), 0);
            return ;
        }
        else if (_channels[ch].isPassSet() && mode == "-k")
        {
            _channels[ch].SetPassFlag(0);
            _channels[ch].setPassword("");
            std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " -k " + arg + "\r\n";
            send(_fds[j].fd, message.c_str(), message.size(), 0);
        }
        else
        {
            if((mode == "+o" || mode == "-o") && arg == "")
            {
                std::string errmsg = _nameserver + " :Devi inserire il nick\r\n";
                send(_fds[j].fd, errmsg.c_str(), errmsg.size(), 0);
            }
            else
            {
                std::string errmsg = _nameserver + " :Password già impostata\r\n";
                send(_fds[j].fd, errmsg.c_str(), errmsg.size(), 0);
            }
            return ;
        }
    }
}
void Server::LeaveClientFormAllChannels(int socketClient)
{

    std::cout << "ENTRO IN LeaveClientFormAllChannels" << std::endl;
    std::map<std::string, Channel>::iterator it;
    std::map<std::string, Channel>::iterator ite;

    it = _channels.begin();
    ite = _channels.end();

    for(; it != ite; it++)
    {
        std::cout << "CICLO I CANALI" << std::endl;
        if(it->second.isClientInChannel(socketClient))
        {
            std::string msg = "#" + it->second.getName() + " :Left channel\r\n";
            goToLeave(msg, socketClient);
            std::cout << "Client " << socketClient << " left channel " << it->second.getName() << std::endl;
        }
    }
    std::cout << "*** ESCO IN LeaveClientFormAllChannels ***" << std::endl;
    return;
}
void Server::goToLeave(std::string argsCmd, int j)
{
    std::string message = argsCmd;
    std::istringstream iss(message);
    std::string ch;
    std::string msg;
    std::string arg = _client[_fds[j].fd].getNick();

    iss >> ch;
    iss >> msg;

    findEraseSpaces(ch);

    std::string messageToSend = ":" + _client[_fds[j].fd].getNick() + "! PART " + ch + " " + msg + "\r\n";
    std::string leaveOperator = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " -o " + arg + "\r\n";
    send(_fds[j].fd, messageToSend.c_str(), messageToSend.size(), 0);
    sendMsgToCh(messageToSend, ch, j);
    _channels[ch].removeSockClient(_fds[j].fd);
    _channels[ch].removeAdmin(_fds[j].fd);
    updateAdminList(leaveOperator, ch);

    if (_channels[ch].getSockClients().size() == 0)
    {
        _channels.erase(ch);
        return ;
    }
    
    std::list<int>::iterator it, tmpit;
    std::list<int>::iterator itr;
    std::list<int>::iterator ite;
    int size;

    it = _channels[ch].getSockClients().begin();
    tmpit = _channels[ch].getSockClients().begin();
    ite = _channels[ch].getSockClients().end();
    size = _channels[ch].getSockClients().size();

    std::string nickList;

    for (; it != ite; ++it)
    {
        if (size == 1)
        {
            if(!_channels[ch].isClientAdmin(*it))
            {
                _channels[ch].setAdmin(*it);
                nickList += "@" + _client[*it].getNick() + " ";
                std::string message = ":" + _client[_fds[j].fd].getNick() + "! MODE " + ch + " +o " + _client[*it].getNick() + "\r\n";
                updateAdminList(message, ch);
            }
            break;
        }
        else if (_channels[ch].isClientAdmin(*it))
            nickList += "@" + _client[*it].getNick() + " ";
        else
            nickList += _client[*it].getNick() + " ";
    }

    it = tmpit;
    std::string namreply;
    namreply = ":" + _nameserver + " 353 " + _client[_fds[j].fd].getNick() + " = " + ch + " :" + nickList + "\r\n";
    std::cout << "***** LISTA UTENTI: " << namreply << std::endl;
    std::string endOfNamesMessage = ":" + _nameserver + " 366 " + _client[_fds[j].fd].getNick() + " " + ch + " :End of /NAMES list\r\n";

    send(_fds[j].fd, namreply.c_str(), namreply.size(), 0);

    send(_fds[j].fd, endOfNamesMessage.c_str(), endOfNamesMessage.size(), 0);
}
void Server::goToWho(std::string argsCmd, int j)
{
    (void)argsCmd;
    (void)j;
    return ;
}
void Server::goToMsg(std::string argsCmd, int j)
{
    std::string message = argsCmd;
    std::istringstream iss(message);
    std::string name;
    iss >> name;
    int isMsgToCh = 0;

    std::cout << "MESSAGE -> " << _client[_fds[j].fd].getNick() << " has sent a message to " << argsCmd << std::endl;
    size_t msg = argsCmd.find(':');
    if(!(msg != std::string::npos))
    {
        std::string message = _nameserver + " ERRORE: IL MESSAGGIO NON E FORMATTATO CORRETTAMENTE" + "\r\n";
        send(_fds[j].fd, message.c_str(), message.size(), 0);
        return ;
    }
    size_t pos = name.find('#');
    if(pos != std::string::npos)
        isMsgToCh = 1;
    if (isMsgToCh && _channels[name].isClientInChannel(_fds[j].fd))
    {
        std::string message = ":" + _client[_fds[j].fd].getNick() + "! PRIVMSG " + name + " " + argsCmd.substr(msg) + "\r\n";
        sendMsgToCh(message, name, j);
        std::cout << "MESSAGE -> sent to " << name << " is >>"<< message << std::endl;

    }
    else if (!isMsgToCh)
    {
        std::string message = ":"+ _client[_fds[j].fd].getNick() + "! PRIVMSG " + name + " " + argsCmd.substr(msg) + "\r\n";
        if(getClientSocket(name) != -1)
        {
            send(getClientSocket(name), message.c_str(), message.size(), 0);
            std::cout << "MESSAGE -> sent to " << name << " is >>"<< message << " and socket is: " << getClientSocket(name) << std::endl;
        }
        else
        {
            std::string error = _nameserver + " :No such nick\r\n";
            send(_fds[j].fd, error.c_str(), error.size(), 0);
            return ;
        }
    }
    else
    {
        std::string error = ":" + _nameserver + " 401 " + _client[_fds[j].fd].getNick() + " " + name + " :No such nick\r\n";
        send(_fds[j].fd, error.c_str(), error.size(), 0);
        return ;
    }

}

void Server::goToQuit(std::string argsCmd, int j)
{
    (void)argsCmd;
    std::cout << "QUIT -> " << _client[_fds[j].fd].getNick() << " has quit" << std::endl;
    LeaveClientFormAllChannels(_fds[j].fd);

    close(_fds[j].fd);
    _fds[j].fd = -1;
    return ;
}

void Server::updateAdminList(std::string message, std::string ch)
{
    for(std::list<int>::iterator it = _channels[ch].getSockClients().begin(); it != _channels[ch].getSockClients().end(); ++it)
        send(*it, message.c_str(), message.size(), 0);
}

void Server::sendMsgToCh(std::string message, std::string ch, int j)
{
    std::list<int>::iterator it = _channels[ch].getSockClients().begin();
    std::list<int>::iterator ite = _channels[ch].getSockClients().end();
    
    for(; it != ite; ++it)
        if(*it != _fds[j].fd)
            send(*it, message.c_str(), message.size(), 0);
}

int Server::getClientSocket(std::string nick)
{
    std::map<int, Client>::iterator it =  _client.begin(); 
    std::map<int, Client>::iterator ite =  _client.end();
    for (; it != ite; it++)
    {
        if (it->second.getNick() == nick)
        {
            std::cout << "*****     NICK TROVATO: " << it->second.getNick() << " NICK CERCATO: " << nick << std::endl;
            return (it->second.getSocket());
        }
    }
    return (-1);
}

void Server::goToInvite(std::string argsCmd, int j)
{
    std::string message = argsCmd;
    std::istringstream iss(message);
    std::string user;
    std::string ch;
    iss >> user;
    iss >> ch;

    std::cout << "INVITE -> " << _client[_fds[j].fd].getNick() << " has invited " << user << " to " << ch << std::endl;

    int ClientToFind = _fds[j].fd;
    std::map<int, Client>::iterator it = _client.begin();
    std::map<int, Client>::iterator ite = _client.end();
    
    for (; it != ite; ++it)
    {
        if(_channels[ch].isClientAdmin(ClientToFind) && it->second.getNick() == user)
            break;
    }
    if(!(it != ite))
    {
        std::string error;
        if(!_channels[ch].isClientAdmin(ClientToFind))
        {
            error = ":" + _nameserver + " 482 " + _client[_fds[j].fd].getNick() + " " + ch + " :You're not channel operator\r\n";
            std::cout << "User Admin: " << _client[ClientToFind].getNick() << " with socket: " << ClientToFind << " is not admin" << std::endl;
        }
        else
        {
            error = _nameserver + " :User not in channel\r\n";
            std::cout << user << " " + it->second.getNick() <<" is not in " << ch << std::endl;
        }
        send(_fds[j].fd, error.c_str(), error.size(), 0);
        return ;
    }

    int clientSocket = getClientSocket(user);
    if (_client[clientSocket].getNick() != user)
    {
        std::string error = ":" + _nameserver + " 401 " + _client[_fds[j].fd].getNick() + " " + user + " :No such nick\r\n";
        send(_fds[j].fd, error.c_str(), error.size(), 0);
        return ;
    }

    _channels[ch].setInvites(clientSocket);
    std::string messageToUser = ":" + _client[_fds[j].fd].getNick() + "! INVITE " + user + " " + ch + "\r\n";
    send(clientSocket, messageToUser.c_str(), messageToUser.size(), 0);
}

void Server::goToKick(std::string argsCmd, int j)
{
    std::string message = argsCmd;
    std::istringstream iss(message);
    std::string ch;
    std::string user;
    std::string msg;
   

    iss >> ch;
    iss >> user;
    while (iss)
    {
        std::string tmp;
        iss >> tmp;
        msg += tmp + " ";
    }
    msg = msg.substr(0, msg.size() - 1);
    
    int ClientToFind = _fds[j].fd;
    std::map<int, Client>::iterator it = _client.begin();
    std::map<int, Client>::iterator ite = _client.end();

    for (; it != ite; ++it)
    {
        if(_channels[ch].isClientAdmin(ClientToFind) && it->second.getNick() == user)
            break;
    }
    if(!(it != ite))
    {
        std::string error;
        if(!_channels[ch].isClientAdmin(ClientToFind))
            error = ":" + _nameserver + " 482 " + _client[_fds[j].fd].getNick() + " " + ch + " :You're not channel operator\r\n";
        else
            error = ":" + _nameserver + " 482 " + _client[_fds[j].fd].getNick() + " " + ch + " :User not in channel\r\n";
        send(_fds[j].fd, error.c_str(), error.size(), 0);
        std::cout << "Errore: " << error << std::endl;
        return ;
    }
    std::string messageToSend = ":" + _client[_fds[j].fd].getNick() + "! KICK " + ch + " " + user + " " + msg + "\r\n";
    std::string messageTokickedUser = ":" + _client[_fds[j].fd].getNick() + " KICK " + ch + " " + user + " " + msg + "\r\n";
    send(_fds[j].fd, messageTokickedUser.c_str(), messageTokickedUser.size(), 0);
    sendMsgToCh(messageToSend, ch, j);
    send(getClientSocket(user), messageTokickedUser.c_str(), messageTokickedUser.size(), 0);
    _channels[ch].removeSockClient(getClientSocket(user));
}

void Server::goToTopic(std::string argsCmd, int j)
{
    std::string message = argsCmd;
    std::istringstream iss(message);
    std::string ch;
    std::string msg;
   

    iss >> ch;
    while (iss)
    {
        std::string tmp;
        iss >> tmp;
        msg += tmp + " ";
    }
    msg = msg.substr(0, msg.size() - 1);

    int ClientToFind = _fds[j].fd;
    std::string error;

    std::cout << "IL CLIENTE N. " << ClientToFind << " E UN AMMINISTRATORE DEL CANALE " << ch << " ? " << _channels[ch].isClientAdmin(ClientToFind) << std::endl;
    if((!_channels[ch].isTopicEnable() || _channels[ch].isClientAdmin(ClientToFind)) && msg != "")
    {
        std::cout << "SONO ENTRATO NEL PRIMO IF" << std::endl;
        _channels[ch].setTopic(msg.substr(1, msg.size()));
        std::string topicMessage = ":" + _nameserver + " 332 " + _client[_fds[j].fd].getNick() + " " + ch + " " + msg + "\r\n";
        std::cout << "topicMessage: " << topicMessage << std::endl;
        send(_fds[j].fd, topicMessage.c_str(), topicMessage.size(), 0);
        sendMsgToCh(topicMessage, ch, j);
    }
    else if(msg == "")
    {
        error = _nameserver + " :Devi inserire il Topic !!!\r\n";
        send(_fds[j].fd, error.c_str(), error.size(), 0);
        std::cout << "Errore: Topic non inserito" << error << std::endl;
    }
    else 
    {
        std::cout << "SONO ENTRATO NEL SECONDO IF" << std::endl;
        std::cout << "IL TOPI E' ABILITATO? " << _channels[ch].isTopicEnable() << std::endl;
        if(!_channels[ch].isClientAdmin(ClientToFind))
            error = ":" + _nameserver + " 482 " + _client[_fds[j].fd].getNick() + " " + ch + " :You're not channel operator\r\n";
        send(_fds[j].fd, error.c_str(), error.size(), 0);
        std::cout << "Errore: " << error << std::endl;
        return ;
    }

}

void Server::clearAll()
{
    for(std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        _channels[it->first].clearAll();
    _channels.clear();
    _client.clear();
}