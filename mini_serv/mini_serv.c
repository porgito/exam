#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

int serverfd;
int clients = 65000;
int SIZE = 400000;

void    putstr(char *str, int fd)
{
    write(fd, str, strlen(str));
    exit(1);
}

int main(int ac, char **av)
{
    if (ac != 2)
        putstr("Wrong number of arguments\n", 2);
    int db[clients], id = 0, fd_size;
    struct sockaddr_in servaddr;
    fd_set old_fd, wr, new_fd;
    char buffer[SIZE], buffer2[SIZE];

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
    fd_size = serverfd;
    FD_ZERO(&old_fd);
    FD_ZERO(&new_fd);
    FD_SET(serverfd, &new_fd);
    while (1)
    {
        old_fd = wr = new_fd;
        if (select(fd_size + 1, &old_fd, &wr, NULL, NULL) < 0)
            putstr("Fatal error\n", 2);
        for(int fd = 0; fd <= fd_size; fd++)
        {
            if (FD_ISSET(fd, &old_fd))
            {
                bzero(&buffer, SIZE);
                bzero(&buffer2, SIZE);
                if (fd == serverfd)
                {
                    struct sockaddr_in claddr;
                    socklen_t len = sizeof(claddr);

                    int clt = accept(serverfd, (struct sockaddr *)&claddr, &len);
                    if (clt < 0)
                        continue;
                    fd_size = (clt > fd_size) ? clt : fd_size;
                    sprintf(buffer, "server: client %d just arrived\n", id);
                    db[clt] = id++;
                    for(int i = 2; i <= fd_size; i++)
                        if (FD_ISSET(i, &wr) && i != serverfd)
                            if (send(i, buffer, strlen(buffer), 0) < 0)
                                putstr("Fatal error\n", 2);
                    FD_SET(clt, &new_fd);
                }
                else if (fd != serverfd)
                {
                    int count = 1;
                    while (count == 1 && buffer2[strlen(buffer2) - 1] != '\n')
                        count = recv(fd, buffer2 + strlen(buffer2), 1, 0);
                    if (count <= 0)
                    {
                        sprintf(buffer, "server: client %d just left\n", db[fd]);
                        FD_CLR(fd, &new_fd);
                        close(fd);
                        for(int i = 2; i <= fd_size; i++)
                            if (FD_ISSET(i, &wr) && i != fd)
                                if (send(i, buffer, strlen(buffer), 0) < 0)
                                    putstr("Fatal error\n", 2);
                    }
                    else
                    {
                        sprintf(buffer, "client %d: %s", db[fd], buffer2);
                        for(int i = 2; i <= fd_size; i++)
                            if (FD_ISSET(i, &wr) && i != serverfd)
                                if (send(i, buffer, strlen(buffer), 0) < 0)
                                    putstr("Fatal error\n", 2);
                    }
                }
                FD_CLR(fd, &old_fd);
            }
        }
    }
    return (0);
}
