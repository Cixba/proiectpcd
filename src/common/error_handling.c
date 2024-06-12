#include "error_handling.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "error_log.h"

//*************************************************
//********************SERVER***********************
//*************************************************

//-------------------------------------------------
//-----------------TCP SOCKET----------------------
//-------------------------------------------------
//socket error
void handle_tcp_socket_error(int err)
{
    fprintf(stderr,"error creating tcp socket\n");
    write_error("tcp bind error");
    exit(EXIT_FAILURE);
}

//set socket options error
void handle_set_tcp_soket_options_error(int err)
{
    fprintf(stderr,"error setting tcp socket options\n");
    exit(EXIT_FAILURE);
}

//tcp bind error
void handle_tcp_bind_error(int err)
{
    fprintf(stderr,"tcp bind error\n");
    exit(EXIT_FAILURE);
}

//tcp_listen_error
void handle_tcp_listen_error(int err)
{
    fprintf(stderr,"tcp listen error\n");
    exit(EXIT_FAILURE);
}

//tcp accept error
void handle_tcp_accept_error(int err)
{
    fprintf(stderr,"tcp accept error\n");
    exit(EXIT_FAILURE);
}

//-------------------------------------------------
//-----------------UNIX SOCKET---------------------
//-------------------------------------------------

//create unix socket error
void handle_unix_socket_error(int err)
{
    switch(err){
        case EACCES:
            fprintf(stderr,"Permission denided\n");
            break;
        case EAFNOSUPPORT:
            fprintf(stderr,"Implementation does not support the address family\n");
            break;
        case EINVAL:
            fprintf(stderr,"Invalid flag or unknown protocol or protocol family not available\n");
            break;
        case EMFILE:
            fprintf(stderr,"Too mach file descriptors\n");
            break;
        case ENFILE:
            fprintf(stderr,"Too much open files\n");
            break;
        case ENOBUFS:
        case ENOMEM:
            fprintf(stderr,"Unsufficient memory available\n");
            break;
        case EPROTONOSUPPORT:
            fprintf(stderr,"Protocol not supported within this domain\n");
            break;
        default:
            fprintf(stderr,"Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}

//set socket options error
void handle_set_unix_soket_options_error(int err)
{
    fprintf(stderr,"error setting unix socket options\n");
    exit(EXIT_FAILURE);
}

//tcp bind error
void handle_unix_bind_error(int err)
{
    switch(err){
        case EACCES:
            fprintf(stderr,"Permission denided\n");
            break;
        case EADDRINUSE:
            fprintf(stderr,"Address already in use\n");
            break;
        case EBADF:
            fprintf(stderr,"Not a valid file descriptor\n");
            break;
        case EINVAL:
            fprintf(stderr,"Socket already bound or addrlen is wrong or addr is not a valid address\n");
            break;
        case ENOTSOCK:
            fprintf(stderr,"The file descriptor does not refer to a socket\n");
            break;
        default:
            fprintf(stderr,"Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}

//tcp_listen_error
void handle_unix_listen_error(int err)
{
    switch(err){
        case EADDRINUSE:
            fprintf(stderr,"Another socket is already listening on the same port\n");
            break;
        case EBADF:
            fprintf(stderr,"The argument sockfd is not a valid file descriptor\n");
            break;
        case ENOTSOCK:
            fprintf(stderr,"sockfd does not refer to a socket\n");
            break;
        case EOPNOTSUPP:
            fprintf(stderr,"The socket is not of a type that supports the listen() operation\n");
            break;

        default:
            fprintf(stderr,"Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}

//tcp accept error
void handle_unix_accept_error(int err)
{
    switch(err){
        case EAGAIN:
            fprintf(stderr,"The socket is marked nonblocking and no connections are present to be accepted\n");
            break;
        case EBADF:
            fprintf(stderr,"sockfd is not an open file descriptor\n");
            break;
        case ECONNABORTED:
            fprintf(stderr,"A connection has been aborted\n");
            break;
        case EFAULT:
            fprintf(stderr,"The addr argument is not in a writable part of the user address space\n");
            break;
        case EINTR:
            fprintf(stderr,"The system call was interrupted by a signal\n");
            break;
        case EINVAL:
            fprintf(stderr,"Socket is not listening for connections, or addrlen is invalid\n");
            break;
        case EMFILE:
            fprintf(stderr,"The per-process limit on the number of open file\n");
            break;
        case ENOBUFS:
        case ENOMEM:
            fprintf(stderr,"Not enough free memory\n");
            break;
        case ENOTSOCK:
            fprintf(stderr,"sockfd does not refer to a socket\n");
            break;
        case EOPNOTSUPP:
            fprintf(stderr,"The referenced socket is not of type SOCK_STREAM\n");
            break;
        default:
            fprintf(stderr,"Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}

//-------------------------------------------------
//--------------------OTHERS-----------------------
//-------------------------------------------------

//select error
void handle_select_error(int err)
{
    fprintf(stderr,"select error\n");
    exit(EXIT_FAILURE);
}

void handle_read_error(int err){
    switch(err){
        case EWOULDBLOCK:
            fprintf(stderr,"The socket is marked nonblocking and the receive operation is block\n");
            break;
        case EBADF:
            fprintf(stderr,"invalid file descriptor\n");
            break;
	    case EFAULT:
            fprintf(stderr,"The receive buffer pointer si outside is address space\n");
            break;
	    case EINTR:
            fprintf(stderr,"Signal intreruption\n");
            break;
	    case EINVAL:
            fprintf(stderr,"Invalid argument passed\n");
            break;
	    case ENOMEM:
            fprintf(stderr,"Could not allocate memory for recvmsg()\n");
            break;
	    case EIO:
            fprintf(stderr,"I/O error\n");
            break;
        default:
            fprintf(stderr,"Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}

void handle_write_error(int err)
{
    switch(err){
        case EACCES:
            fprintf(stderr,"Write permission is denied\n");
            break;
        case EAGAIN:
            fprintf(stderr,"All port numbers are currently in use\n");
            break;
	    case EBADF :
            fprintf(stderr,"invalid file descriptor\n");
            break;
	    case EFAULT:
            fprintf(stderr,"Invalid user space address\n");
	        break;
	    case EINTR:
            fprintf(stderr,"A signal interruption\n");
	        break;
	    case EINVAL:
            fprintf(stderr,"Invalid argument passed\n");
	        break;
	    case EPIPE:
            fprintf(stderr,"The local end has been shut down on a connection\n");
	        break;
        default:
            fprintf(stderr,"Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}

//open error
void handle_open_error(int err)
{
    switch(err)
    {
        case EACCES://nu sunt permisiuni necesare
            fprintf(stderr,"open error: Permission denide or requested access not allowed\n");
            break;
        case EEXIST://path-ul fisierului exista
            fprintf(stderr,"open error: Path already exists\n");
            break;
        case EFAULT://path-ul pointeaza spre o zona de memorie ce nu poate fi accesata
            fprintf(stderr,"open error: Path points outside accessible address space\n");
            break;
        case ELOOP://prea multe symbolic link-uri in path
            fprintf(stderr,"open error: Too many symbolic links in path\n");
            break;
        case ENAMETOOLONG://path-ul e prea lung
            fprintf(stderr,"open error: Path too long\n");
            break;
        case ENOENT://in caz ca un director din path nu exista sau e un link simbolic pentru care fisierul tinta nu mai exista
            fprintf(stderr,"open error: Not found\n");
            break;
        case ENOMEM://memorie kernel insuficienta
            fprintf(stderr,"open error: Insufficient kernel memory available\n");
            break;
        case EINVAL://invalid flag 
            fprintf(stderr,"open error: Invalid flag\n");
            break;
        case ENFILE: //limita maxima de fisiere deschise a fost atinsa
            fprintf(stderr,"open error: Too many open files\n");
            break;
        default://alta eroare
            fprintf(stderr,"open error: Some error occured\n");
            break;
    }
    exit(EXIT_FAILURE);
}

//dup2 error
void handle_dup2_error(int err)
{
    fprintf(stderr,"dup2 error\n");
    exit(EXIT_FAILURE);
}

void handle_fork_error(int err)
{
    fprintf(stderr,"fork error\n");
    exit(EXIT_FAILURE);
}

//user type error
void handle_unknown_user(int err)
{
    fprintf(stderr,"unknown user error\n");
    exit(EXIT_FAILURE);
}

//*************************************************
//********************CLIENT***********************
//*************************************************

//tcp connect error
void handle_tcp_connect_error(int err)
{
    fprintf(stderr,"tcp connect error\n");
    exit(EXIT_FAILURE);
}

//unix connect error
void handle_unix_connect_error(int err)
{
    fprintf(stderr,"unix connect error: ");
    switch(err){
        case EACCES:
            fprintf(stderr,"Write permission is denied on the socket file\n");
            break;
        case EPERM:
            fprintf(stderr,"Connection request failed because of a local firewall rule\n");
            break;
        case EADDRINUSE:
            fprintf(stderr,"Local address is already in use\n");
            break;
        case EADDRNOTAVAIL:
            fprintf(stderr,"sockfd had not previously been bound to an address\n");
            break;
        case EAFNOSUPPORT:
            fprintf(stderr,"The passed address didn't have the correct address family\n");
            break;
        case EAGAIN:
            fprintf(stderr,"The socket is nonblocking, and the connection cannot be completed immediately.\n");
            break;
        case EALREADY:
            fprintf(stderr,"The socket is nonblocking and a previous connection attempt has not yet been completed\n");
            break;
        case EBADF:
            fprintf(stderr,"sockfd is not a valid open file descriptor\n");
            break;
        case ECONNREFUSED:
            fprintf(stderr,"A connect() on a stream socket found no one listening on the remote address\n");
			break;
        case EFAULT:
            fprintf(stderr,"A connect() on a stream socket found no one listening on the remote address\n");
			break;
        case EINPROGRESS:
            fprintf(stderr,"The socket is nonblocking and the connection cannot be completed immediately\n");
			break;
        case EINTR:
            fprintf(stderr,"The system call was interrupted by a signal that was caught\n");
			break;
        case EISCONN:
            fprintf(stderr,"The socket is already connected\n");
			break;
        case ENETUNREACH:
            fprintf(stderr,"Network is unreachable\n");
			break;
        case ENOTSOCK:
            fprintf(stderr,"sockfd does not refer to a socket\n");
			break;
        case EPROTOTYPE:
            fprintf(stderr,"The socket type does not support the requested communications protocol\n");
			break;
        case ETIMEDOUT:
            fprintf(stderr,"Timeout while attempting connection\n");
			break;
        default:
            fprintf(stderr,"Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}