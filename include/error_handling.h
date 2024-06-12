#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

//-------------------------------------------------
//--------------------SERVER-----------------------
//-------------------------------------------------

void handle_tcp_socket_error(int err);
void handle_set_tcp_soket_options_error(int err);
void handle_tcp_bind_error(int err);
void handle_tcp_listen_error(int err);
void handle_tcp_accept_error(int err);

void handle_unix_socket_error(int err);
void handle_set_unix_soket_options_error(int err);
void handle_unix_bind_error(int err);
void handle_unix_listen_error(int err);
void handle_unix_accept_error(int err);

void handle_select_error(int err);

void handle_read_error(int err);
void handle_write_error(int err);
void handle_open_error(int err);
void handle_dup2_error(int err);
void handle_unknown_user(int err);

void handle_fork_error(int err);

//-------------------------------------------------
//--------------------CLIENT-----------------------
//-------------------------------------------------

void handle_tcp_connect_error(int err);
void handle_unix_connect_error(int err);

#endif