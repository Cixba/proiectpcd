#ifndef UNIX_H
#define UNIX_H

int create_unix_socket();
int accept_unix_connection(const int sockfd);

#endif