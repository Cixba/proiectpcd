#include <sys/socket.h>     //socket
#include <arpa/inet.h>      //struct sockaddr_in
#include <errno.h>          //pentru errno
#include <string.h>         //strlen, strcmp, strcpy

#include "error_handling.h"
#include "constants.h"

static int conn_limit = 10;

//-------------------------------------------------
//----------------------TCP------------------------
//-------------------------------------------------

//functia primeste ca argumenst un socket descriptor
//si il seteaza sa poata fi folosit pentru mai multe conexiuni simultane
static int make_reusable(int fd)
{
    int reuse = 1U;

    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&reuse, sizeof(reuse)) == -1) handle_set_tcp_soket_options_error(errno);

    return fd;
}

//functia creaza socketul peste TCP
//prin care se vor conecta clientii
int create_tcp_socket()
{
    int sockfd;
    struct sockaddr_in serv_addr;

    //creare socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) handle_tcp_socket_error(errno);

    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ADDR);
    serv_addr.sin_port = htons(PORT);

    //bind
    sockfd = make_reusable(sockfd);
    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) handle_tcp_bind_error(errno);
    
    if((listen(sockfd, conn_limit)) != 0) handle_tcp_listen_error(errno);

    return sockfd;
}

//functia asteapta conexiunea unui client
int accept_tcp_connection(const int sockfd)
{
    struct sockaddr_in cli_addr;    //client
    int newsockfd;
    bzero((char *) &cli_addr, sizeof(cli_addr));
    unsigned int len = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &len);
       
    if (newsockfd < 0) handle_tcp_accept_error(errno);

    return newsockfd;
}