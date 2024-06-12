#ifndef HANDLE_ADMIN_H
#define HANDLE_ADMIN_H

#include "connection.h"     //struct connection



void handle_admin_connection(struct connection conn);
void handle_view_logs_request(int sockfd);

#endif