#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

typedef struct s_client
{
    int id;
    char msg[1024];
} t_client;

t_client clients[1024];
fd_set read_set, write_set, current;
int fd_size, gid = 0;
char buffer[400000], buffer2[450000];

void    err(char *str)
{
    if (str)
        write(2, str, strlen(str));
    else
        write(2, "Fatal error", 11);
    write(2, "\n", 1);
    exit(1);
}

void    sendall(int except)
{
    for(int fd = 0; fd <= fd_size; fd++)
        if (FD_ISSET(fd, &write_set) && fd != except)
            if (send(fd, buffer, strlen(buffer), 0) == -1)
                err(NULL);
}

int main(int ac, char **av)
{
    if (ac != 2)
        err("Wrong number of arguments");
    struct sockaddr_in servaddr;
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd == -1)
        err(NULL);
    fd_size = serverfd;
    FD_ZERO(&current);
    FD_SET(serverfd, &current);

    bzero(clients, sizeof(clients));
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(2130706433);
    servaddr.sin_port = htons(atoi(av[1]));

    if (bind(serverfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == -1 || listen(serverfd, 100) == -1)
        err(NULL);
    
    while (1)
    {
        read_set = write_set = current;
        if (select(fd_size + 1, &read_set, &write_set, NULL, NULL) == -1)
            continue;
        for(int fd = 0; fd <= fd_size; fd++)
        {
            bzero(&buffer, 400000);
            bzero(&buffer2, 450000);
            bzero(&clients[fd].msg, strlen(clients[fd].msg));

            if (FD_ISSET(fd, &read_set))
            {
                if (fd == serverfd)
                {
                    int clientfd = accept(serverfd, NULL, NULL);
                    if (clientfd == -1)
                        continue;
                    if (fd_size < clientfd)
                        fd_size = clientfd;
                    clients[clientfd].id = gid++;
                    FD_SET(clientfd, &current);
                    sprintf(buffer, "server: client %d just arrived\n", clients[clientfd].id);
                    sendall(clientfd);
                    break;
                }
                else
                {
                    int ret = 1;
                    while (ret == 1 && buffer2[strlen(buffer2) - 1] != '\n')
                        ret = recv(fd, buffer2 + strlen(buffer2), 1, 0);
                    if (ret <= 0)
                    {
                        sprintf(buffer, "server: client %d just left\n", clients[fd].id);
                        sendall(fd);
                        FD_CLR(fd, &current);
                        close(fd);
                    }
                    else
                    {   
                        sprintf(buffer, "client %d: %s", clients[fd].id, buffer2);
                        sendall(serverfd);
                    }
                }
            }
            FD_CLR(fd, &read_set);
        }
    }
}