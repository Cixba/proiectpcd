#include <stdio.h>
#include <stdlib.h>         //exit()
#include <unistd.h>         //open
#include <string.h>         //strlen, strcmp, strcpy
#include <sys/wait.h>       //wait
#include <sys/socket.h>     //socket
#include <arpa/inet.h>      //struct sockaddr_in
#include <time.h>           //time, localtime
#include <errno.h>          //pentru errno
#include <sys/un.h>         //struct sockaddr_un
#include <sys/select.h>

#include "variables.h"          //variabile globale
#include "error_handling.h"     //tratare erori
#include "constants.h"          //constante
#include "tcp.h"                //socket tcp
#include "unix.h"               //socket unix
#include "log.h"                //logs
#include "handle_admin.h"
#include "handle_client.h"

time_t start_time;  //timpul de start
int tcp_sockfd;
int unix_sockfd;
int pid;            //server pid

//creare nou proces pentru clientul conectat
void create_new_process(struct connection conn)
{
    //fprintf(stdout,"in create new process\n");
    switch(fork())
    {
        case -1://eroare
            handle_fork_error(errno);
            break;
        case 0://copil
            time_t login_time;
            struct tm *local_time;
            char log[64];
            switch(conn.type)
            {
                case ADMIN:
                    sprintf(log, "Admin PID[%d] conectat", conn.pid);
                    write_log(log);

                    handle_admin_connection(conn);
                    break;
                case SIMPLE:
                    sprintf(log, "Client PID[%d] conectat", conn.pid);
                    write_log(log);

                    handle_client_connection(conn);
                    break;
                default:
                    handle_unknown_user(errno);
                    break;
            }
            break;
        default://parinte

            break;
    }
}

//sigint, sigtstp handler
void handle_sigint_sigtstp(int sig)                        
{
    //informare clienti ca serverul nu mai ruleaza
    
    if(pid == getpid())
    {
        write_log("Serverul se opreste");
        fprintf(stdout,"             \n");
        char log[64];
        for(int i=0;i<connections_index;i++)
        {
            sprintf(log,"Clientul PID[%d] a fost deconectat de catre server",connections[i].pid);
            write_log(log);
            //trimit semnalul SIGTERM spre client
            kill(connections[i].pid,SIGTERM);
            //inchide socket ul
            close(connections[i].sockfd);
        }

        //resetam numarul de clienti conectati
        connections_index = 0;

        close(tcp_sockfd);
        close(unix_sockfd);
        write_log("Serverul s-a oprit");
    }

    exit(EXIT_SUCCESS);
}

//functia citeste pid-ul clientului conectat trimis 
//odata ce conectiunea este stabilita
int get_connection_pid(int sockfd)
{
    char str[10];
    int pid = -1;
    if(read(sockfd, str, 10) < 0) handle_read_error(errno);
    sscanf(str, "%d",&pid);

    return pid;
}

int is_admin_connected()
{
    for(int i=0;i<connections_index;i++)
        {
            if(connections[i].type == ADMIN) return 1;
            fprintf(stdout,"%d\n",connections[i].type );
        }
    
    return 0;
}

int main(int argc, char* argv[])
{
	
    int /*tcp_sockfd, unix_sockfd,*/clientfd,adminfd;
    int parent = 0;

    pid = getpid();

    signal(SIGINT, handle_sigint_sigtstp);
    signal(SIGTSTP, handle_sigint_sigtstp);

    time(&start_time);  //timpul de start
    
    open_log_file();    //fisier logs

    write_log("Serverul ruleaza");

    //creare socket pentru clienti
    tcp_sockfd = create_tcp_socket();

    //creare socket administratori
    unix_sockfd = create_unix_socket();

    //pentru verificarea tipului de conexiune
    fd_set readfds;
    int max;
    int current_con_pid;
    int i=0;

    for(;;)
    {//start_time
        FD_ZERO(&readfds);
        FD_SET(tcp_sockfd, &readfds);
        FD_SET(unix_sockfd, &readfds);

        max = tcp_sockfd > unix_sockfd ? tcp_sockfd : unix_sockfd;
        
        //blocam pana avem o cerere de conexiune
        int select_result = select(max + 1, &readfds, NULL, NULL, NULL);

        if(select_result < 0) handle_select_error(errno);

        //verificam conexiunea cu un client
        if(FD_ISSET(tcp_sockfd, &readfds))
        {
            //acceptam conexiunea cu un client
            clientfd = accept_tcp_connection(tcp_sockfd);
            current_con_pid = get_connection_pid(clientfd);

            //salvez noua conexiune
            struct connection new_connection;
            new_connection.pid = current_con_pid;
            new_connection.sockfd = clientfd;
            new_connection.type = SIMPLE;
            connections[connections_index++] = new_connection;

            //cream un nou procespentru conexiunea stabilita
            create_new_process(new_connection);
        }

        //verificam conexiunea cu un administrator
        if(FD_ISSET(unix_sockfd, &readfds))
        {
            //acceptam conexiunea cu un adminitrator
            //admin_connected
            if(is_admin_connected())
            {
                fprintf(stdout, "Un administrator este deja conectat\n",i);
            }
            adminfd = accept_unix_connection(unix_sockfd);
            current_con_pid = get_connection_pid(adminfd);
            
            //salvez noua conexiune
            struct connection new_connection;
            new_connection.pid = current_con_pid;
            new_connection.sockfd = adminfd;
            new_connection.type = ADMIN;
            connections[connections_index++] = new_connection;

            
            //cream un nou procespentru conexiunea stabilita
            create_new_process(new_connection);
            
        }
        i++;

    }

}
