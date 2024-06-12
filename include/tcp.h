#ifndef TCP_H
#define TCP_H

int make_reusable(int fd);
int create_tcp_socket();
int accept_tcp_connection(const int sockfd);

#endif