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
    write_error("error creating tcp socket\n");
    write_error("tcp bind error");
    //exit(EXIT_FAILURE);
}

//set socket options error
void handle_set_tcp_soket_options_error(int err)
{
    write_error("error setting tcp socket options\n");
    exit(EXIT_FAILURE);
}

//tcp bind error
void handle_tcp_bind_error(int err)
{
    write_error("tcp bind error\n");
    exit(EXIT_FAILURE);
}

//tcp_listen_error
void handle_tcp_listen_error(int err)
{
    write_error("tcp listen error\n");
    exit(EXIT_FAILURE);
}

//tcp accept error
void handle_tcp_accept_error(int err)
{
    write_error("tcp accept error\n");
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
            write_error("Permission denided\n");
            break;
        case EAFNOSUPPORT:
            write_error("Implementation does not support the address family\n");
            break;
        case EINVAL:
            write_error("Invalid flag or unknown protocol or protocol family not available\n");
            break;
        case EMFILE:
            write_error("Too mach file descriptors\n");
            break;
        case ENFILE:
            write_error("Too much open files\n");
            break;
        case ENOBUFS:
        case ENOMEM:
            write_error("Unsufficient memory available\n");
            break;
        case EPROTONOSUPPORT:
            write_error("Protocol not supported within this domain\n");
            break;
        default:
            write_error("Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}

//set socket options error
void handle_set_unix_soket_options_error(int err)
{
    write_error("error setting unix socket options\n");
    exit(EXIT_FAILURE);
}

//tcp bind error
void handle_unix_bind_error(int err)
{
    switch(err){
        case EACCES:
            write_error("Permission denided\n");
            break;
        case EADDRINUSE:
            write_error("Address already in use\n");
            break;
        case EBADF:
            write_error("Not a valid file descriptor\n");
            break;
        case EINVAL:
            write_error("Socket already bound or addrlen is wrong or addr is not a valid address\n");
            break;
        case ENOTSOCK:
            write_error("The file descriptor does not refer to a socket\n");
            break;
        default:
            write_error("Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}

//tcp_listen_error
void handle_unix_listen_error(int err)
{
    switch(err){
        case EADDRINUSE:
            write_error("Another socket is already listening on the same port\n");
            break;
        case EBADF:
            write_error("The argument sockfd is not a valid file descriptor\n");
            break;
        case ENOTSOCK:
            write_error("sockfd does not refer to a socket\n");
            break;
        case EOPNOTSUPP:
            write_error("The socket is not of a type that supports the listen() operation\n");
            break;

        default:
            write_error("Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}

//tcp accept error
void handle_unix_accept_error(int err)
{
    switch(err){
        case EAGAIN:
            write_error("The socket is marked nonblocking and no connections are present to be accepted\n");
            break;
        case EBADF:
            write_error("sockfd is not an open file descriptor\n");
            break;
        case ECONNABORTED:
            write_error("A connection has been aborted\n");
            break;
        case EFAULT:
            write_error("The addr argument is not in a writable part of the user address space\n");
            break;
        case EINTR:
            write_error("The system call was interrupted by a signal\n");
            break;
        case EINVAL:
            write_error("Socket is not listening for connections, or addrlen is invalid\n");
            break;
        case EMFILE:
            write_error("The per-process limit on the number of open file\n");
            break;
        case ENOBUFS:
        case ENOMEM:
            write_error("Not enough free memory\n");
            break;
        case ENOTSOCK:
            write_error("sockfd does not refer to a socket\n");
            break;
        case EOPNOTSUPP:
            write_error("The referenced socket is not of type SOCK_STREAM\n");
            break;
        default:
            write_error("Other error occured\n");
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
    write_error("select error\n");
    exit(EXIT_FAILURE);
}

void handle_read_error(int err){
    switch(err){
        case EWOULDBLOCK:
            write_error("The socket is marked nonblocking and the receive operation is block\n");
            break;
        case EBADF:
            write_error("invalid file descriptor\n");
            break;
	    case EFAULT:
            write_error("The receive buffer pointer si outside is address space\n");
            break;
	    case EINTR:
            write_error("Signal intreruption\n");
            break;
	    case EINVAL:
            write_error("Invalid argument passed\n");
            break;
	    case ENOMEM:
            write_error("Could not allocate memory for recvmsg()\n");
            break;
	    case EIO:
            write_error("I/O error\n");
            break;
        default:
            write_error("Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}

void handle_write_error(int err)
{
    switch(err){
        case EACCES:
            write_error("Write permission is denied\n");
            break;
        case EAGAIN:
            write_error("All port numbers are currently in use\n");
            break;
	    case EBADF :
            write_error("invalid file descriptor\n");
            break;
	    case EFAULT:
            write_error("Invalid user space address\n");
	        break;
	    case EINTR:
            write_error("A signal interruption\n");
	        break;
	    case EINVAL:
            write_error("Invalid argument passed\n");
	        break;
	    case EPIPE:
            write_error("The local end has been shut down on a connection\n");
	        break;
        default:
            write_error("Other error occured\n");
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
            write_error("open error: Permission denide or requested access not allowed\n");
            break;
        case EEXIST://path-ul fisierului exista
            write_error("open error: Path already exists\n");
            break;
        case EFAULT://path-ul pointeaza spre o zona de memorie ce nu poate fi accesata
            write_error("open error: Path points outside accessible address space\n");
            break;
        case ELOOP://prea multe symbolic link-uri in path
            write_error("open error: Too many symbolic links in path\n");
            break;
        case ENAMETOOLONG://path-ul e prea lung
            write_error("open error: Path too long\n");
            break;
        case ENOENT://in caz ca un director din path nu exista sau e un link simbolic pentru care fisierul tinta nu mai exista
            write_error("open error: Not found\n");
            break;
        case ENOMEM://memorie kernel insuficienta
            write_error("open error: Insufficient kernel memory available\n");
            break;
        case EINVAL://invalid flag 
            write_error("open error: Invalid flag\n");
            break;
        case ENFILE: //limita maxima de fisiere deschise a fost atinsa
            write_error("open error: Too many open files\n");
            break;
        default://alta eroare
            write_error("open error: Some error occured\n");
            break;
    }
    exit(EXIT_FAILURE);
}

//dup2 error
void handle_dup2_error(int err)
{
    write_error("dup2 error\n");
    exit(EXIT_FAILURE);
}

void handle_fork_error(int err)
{
    write_error("fork error\n");
    exit(EXIT_FAILURE);
}

//user type error
void handle_unknown_user(int err)
{
    write_error("unknown user error\n");
    exit(EXIT_FAILURE);
}

//*************************************************
//********************CLIENT***********************
//*************************************************

//tcp connect error
void handle_tcp_connect_error(int err)
{
    //fprintf(stdout,"tcp connect error\n");
    switch(err)
    {
        case ECONNREFUSED:
            fprintf(stdout,"Serverul este oprit\n");
			break;
        case EFAULT:
            fprintf(stdout,"Serverul este oprit\n");
			break;
    }
    exit(EXIT_FAILURE);
}

//unix connect error
void handle_unix_connect_error(int err)
{
    //write_error("unix connect error: ");
    switch(err){
        case EACCES:
            fprintf(stdout,"Write permission is denied on the socket file\n");
            break;
        case EPERM:
            fprintf(stdout,"Connection request failed because of a local firewall rule\n");
            break;
        case EADDRINUSE:
            fprintf(stdout,"Local address is already in use\n");
            break;
        case EADDRNOTAVAIL:
            fprintf(stdout,"sockfd had not previously been bound to an address\n");
            break;
        case EAFNOSUPPORT:
            fprintf(stdout,"The passed address didn't have the correct address family\n");
            break;
        case EAGAIN:
            fprintf(stdout,"The socket is nonblocking, and the connection cannot be completed immediately.\n");
            break;
        case EALREADY:
            fprintf(stdout,"The socket is nonblocking and a previous connection attempt has not yet been completed\n");
            break;
        case EBADF:
            fprintf(stdout,"sockfd is not a valid open file descriptor\n");
            break;
        case ECONNREFUSED:
            fprintf(stdout,"Serverul este oprit\n");
			break;
        case EFAULT:
            fprintf(stdout,"Serverul este oprit\n");
			break;
        case EINPROGRESS:
            fprintf(stdout,"The socket is nonblocking and the connection cannot be completed immediately\n");
			break;
        case EINTR:
            fprintf(stdout,"The system call was interrupted by a signal that was caught\n");
			break;
        case EISCONN:
            fprintf(stdout,"The socket is already connected\n");
			break;
        case ENETUNREACH:
            fprintf(stdout,"Network is unreachable\n");
			break;
        case ENOTSOCK:
            fprintf(stdout,"sockfd does not refer to a socket\n");
			break;
        case EPROTOTYPE:
            fprintf(stdout,"The socket type does not support the requested communications protocol\n");
			break;
        case ETIMEDOUT:
            fprintf(stdout,"Timeout while attempting connection\n");
			break;
        default:
            fprintf(stdout,"Other error occured\n");
            break;
        }
    exit(EXIT_FAILURE);
}