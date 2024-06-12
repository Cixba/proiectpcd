#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include "connection.h"
#include "error_handling.h"
#include "constants.h"

struct connection connections[20];
int connections_index = 0;

//functia citeste datele trimite de catre un client
int read_from_client(int sockfd, char *str)
{
    int bytes_read = 0;

    if((bytes_read = read(sockfd, str, BUFFER_SIZE)) < 0) handle_read_error(errno);
    str[bytes_read] = '\0';

    return bytes_read;
}

//functia trimite datele prelucrate spre client
int write_to_client(int sockfd, char *data)
{
    int bytes_write = 0;
    if((bytes_write = write(sockfd,data, strlen(data))) < 0) handle_write_error(errno);

    return bytes_write;
}

//functia deconecteaza un client conectat la server
void disconnect_client(struct connection conn)
{
    int position = 0;
    //cautam conexiunea
    for(int i=0;i<connections_index;i++)
    {
        if(connections[i].pid == conn.pid)
        {
            position = i;
            break;
        }
    }

    //oprim clientul 
    kill(conn.pid, SIGUSR1);

    //stergem conexiunea
    if(position)
    {
        for(int i=position;i<connections_index-1;i++)
        {
            connections[i] = connections[i+1];
        }

        connections_index--;
    }
}