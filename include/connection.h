#ifndef CONNECTION_H
#define CONNECTION_H

typedef enum{
    SIMPLE,
    ADMIN,
    UNKNOWN
}ClientType;

struct connection
{
    int sockfd;
    int pid;
    ClientType type;
};

extern struct connection connections[];
extern int connections_index;

int read_from_client(int sockfd, char *str);
int write_to_client(int sockfd, char *data);
void disconnect_client(struct connection conn);

#endif