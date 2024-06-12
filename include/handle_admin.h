#ifndef HANDLE_ADMIN_H
#define HANDLE_ADMIN_H

#include "connection.h"     //struct connection
/*
typedef enum{
    SIMPLE,
    ADMIN,
    UNKNOWN
}ClientType;

//ClientType clientType = UNKNOWN;    //initial necunoscut

struct connection
{
    int sockfd;
    int pid;
    ClientType type;
};*/

void handle_admin_connection(struct connection conn);
//int read_from_client(int sockfd, char *str);
//int write_to_client(int sockfd, char *data);
void handle_view_logs_request(int sockfd);

#endif