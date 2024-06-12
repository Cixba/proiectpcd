#include <unistd.h>         //getlogin()
#include <sys/socket.h>     //socket
#include <errno.h>          //pentru errno
#include <sys/un.h>         //struct sockaddr_un
#include <string.h>         //strlen, strcmp, strcpy

#include "error_handling.h"
#include "constants.h"

//-------------------------------------------------
//----------------------UNIX-----------------------
//-------------------------------------------------

//functia creaza socketul prin care se vor conecta administratorii
int create_unix_socket()
{
    int sockfd;
    struct sockaddr_un serv_addr;

    //unlink socket
    unlink(UNIX_SOCKET_PATH);

    //creare socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd == -1) handle_unix_socket_error(errno);

    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, UNIX_SOCKET_PATH);

    //asignare adresa
    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) handle_unix_bind_error(errno);

    if(listen(sockfd,1) < 0) handle_unix_listen_error(errno); 

    return sockfd;
}

//functia asteapta si accepta conexiunea cu un administrator
int accept_unix_connection(const int sockfd)
{
    struct sockaddr_un admin_addr;  //admin
    int newsockfd;
    bzero((char *) &admin_addr, sizeof(admin_addr));
    unsigned int len = sizeof(admin_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &admin_addr, &len);
       
    if (newsockfd < 0) handle_unix_accept_error(errno);

    return newsockfd;
}