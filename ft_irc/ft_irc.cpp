#include "server.hpp"
#include "client.hpp"
#include "channel.hpp"

Server server("", 0);

void signalHandler(int signum)
{
    std::cout << "Ricevuto segnale SIGINT (Ctrl+C). Sblocco la porta del server e termino.\n";
    for (int i = 0; i < server.getMaxClients(); i++)
    {
        if (server.getFds(i) != -1)
            close(server.getFds(i));
    }
    //server.clearAll();
    server.clearPartialCommand();
    close(server.getServerSock());
    exit(signum);
}

void signalFuck(int signum)
{
    std::cout << "Ricevuto segnale SIGSEGV. Sblocco la porta del server e termino.\n";
    for (int i = 0; i < server.getMaxClients(); i++)
    {
        if (server.getFds(i) != -1)
            close(server.getFds(i));
    }
    //server.clearAll();
    server.clearPartialCommand();
    close(server.getServerSock());
    exit(signum);
}

int main(int ac, char **av)
{
    signal(SIGINT, signalHandler);
    signal(SIGSEGV, signalFuck);

    int socket;

    if (ac != 3)
    {
        std::cout << "Usage: ./ft_irc [port] [pass]" << std::endl;
        return (0);
    }
    try
    {
        ft_stoi(av[1]);
        if(ft_stoi(av[1]) <= 1024 || ft_stoi(av[1]) >= 65534)
        {
            std::cerr << "Non e' un numero di porta valido" << std::endl;
            return (0);
        }
    }
    catch(...)
    {
        std::cerr << "Port must be a number" << std::endl;
        return (0);
    }
    server.setPass(av[2]);
    server.setPort(ft_stoi(av[1]));    
    socket = server.CreateSocketConnection();
    if(listen(socket, server.getMaxClients()) == -1)
    {
        std::cerr << "Errore nella listen" << std::endl;
        return (0);
    }
    server.setServerSock(socket);
    server.ServerRun();
}