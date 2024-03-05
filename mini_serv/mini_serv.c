#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <netdb.h>

int serverfd;
int clients = 65000;
int SIZE = 450000;

void    putstr(char *str, int fd)
{
    write(fd, str, strlen(str));
    exit(1);
}

int main(int ac, char **av)
{
    if (ac != 2)
        putstr("Wrong number of arguments\n", 2);
    struct sockaddr_in servaddr;
    fd_set new_fd, ready, wr;
    int id = 0;
    int arr[clients];
    char buffer[SIZE], msg[SIZE];

    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0)
        putstr("Fatal error\n", 2);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(2130706433);
    servaddr.sin_port = htons(atoi(av[1]));
    if (bind(serverfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        putstr("Fatal error\n", 2);
    if (listen(serverfd, 100))
        putstr("Fatal error\n", 2);
    int fd_size = serverfd;
    FD_ZERO(&new_fd);
    FD_SET(serverfd, &new_fd);
    while (1)
    {
        ready = wr = new_fd;
        if (select(fd_size + 1, &ready, &wr, NULL, NULL) < 0)
            continue;
        for(int fd = 0; fd <= fd_size; fd++)
        {
            if (FD_ISSET(fd, &ready))
            {
                bzero(&msg, strlen(msg));
                bzero(&buffer, strlen(msg));
                if (fd == serverfd)
                {
                    struct sockaddr_in claddr;
                    socklen_t len = sizeof(claddr);

                    int clt = accept(serverfd, (struct sockaddr*)&claddr, &len);
                    if (clt < 0)
                        continue;
                    fd_size = (clt > fd_size) ? clt : fd_size;
                    sprintf(buffer, "server: client %d just arrived\n", id);
                    arr[clt] = id++;
                    for(int j = 2; j <= fd_size; j++)
                    {
                        if (FD_ISSET(j, &wr) && j != serverfd)
                        {
                            if (send(j, buffer, strlen(buffer), 0) < 0)
                                putstr("Fatal error\n", 2);
                        }
                    }
                    FD_SET(clt, &new_fd);
                }
                else if (fd != serverfd)
                {
                    int count = 1;
                    while (count == 1 && msg[strlen(msg) - 1] != '\n')
                        count = recv(fd, msg + strlen(msg), 1, 0);
                    if (count <= 0)
                    {
                        sprintf(buffer, "server: client %d just left\n", arr[fd]);
                        FD_CLR(fd, &new_fd);
                        close(fd);
                        for(int z = 2; z <= fd_size; z++)
                        {
                            if (FD_ISSET(z, &wr) && z != fd)
                            {
                                if (send(z, buffer, strlen(buffer), 0) < 0)
                                {
                                    putstr("Fatal error\n", 2);
                                }
                            }
                        }
                    }
                    else
                    {
                        sprintf(buffer, "client %d: %s", arr[fd], msg);
                        for(int z = 2; z <= fd_size; z++)
                            if (FD_ISSET(z, &wr) && z != serverfd)
                                if (send(z, buffer, strlen(buffer), 0) < 0)
                                    putstr("Fatal error\n", 2);
                    }
                }
                FD_CLR(fd, &ready);
            }
        }
    }
    return (0); 
}
