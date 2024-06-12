#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>

#include "handle_admin.h"
#include "constants.h"
#include "log.h"
#include "error_handling.h"

//functia trimite fisierul de logs catre administrator
void handle_view_logs_request(int sockfd)
{
    char str[BUFFER_SIZE], copy[BUFFER_SIZE];
    int fd, bytes_read;
    DIR *logs;
    struct dirent *entry;
    char path[BUFFER_SIZE + 100];
    //deschid folderul de logs si preiau numele fisierelor
    logs = opendir(LOG_DIR_PATH);
    while((entry = readdir(logs)))
    {
        if(strstr(entry->d_name,"log"))
        {
            strcpy(str + strlen(str), entry->d_name);
            strcpy(str + strlen(str), "\n");
        }
    }

    closedir(logs);

    str[strlen(str)] = '\0';
    strcpy(copy, str);

    //trimit lista de fisire la admin
    write_to_client(sockfd,str);
    str[0] = '\0';

    //astept fisierul ales de admin
    read_from_client(sockfd,str);
    fprintf(stdout,"%s\n",str);
    //formez calea fisierului
    sprintf(path + 0, "%s%s",LOG_PATH,str);
    fprintf(stdout,"%s\n",path);

    //deschid fisierul de log pentru citire
    if((fd = open(path, O_RDONLY)) < 0) handle_open_error(errno);
    //trimit fisierul spre admin
    while((bytes_read = read(fd, str, BUFFER_SIZE)) > 0)
    {
        if(bytes_read < strlen(str)) str[bytes_read] = '\0';
        if(write(sockfd, str, strlen(str)) < 0) handle_write_error(errno);
    }
    //trimit finalul de masaj
    close(fd);
    if(write(sockfd, EOM, strlen(EOM)) < 0) handle_write_error(errno);
}

//functia trimite fisierul de loerori catre administrator
void handle_view_errors_request(int sockfd)
{
    char str[BUFFER_SIZE];
    int fd, bytes_read;

    //deschid fisierul de log pentru citire
    if((fd = open(ERROR_PATH, O_RDONLY)) < 0) handle_open_error(errno);

    //trimit fisierul spre admin
    while((bytes_read = read(fd, str, BUFFER_SIZE)) > 0)
    {
        if(bytes_read < strlen(str)) str[bytes_read] = '\0';
        if(write(sockfd, str, strlen(str)) < 0) handle_write_error(errno);
    }
    close(fd);
    //trimit finalul de masaj
    if(write(sockfd, EOM, strlen(EOM)) < 0) handle_write_error(errno);
}

//functia triite lista de conexiuni active cu serverul
void send_clients_list_to_admin(struct connection conn)
{
    char resp[BUFFER_SIZE];
    char log[64];
    resp[0] = '\0';
    for(int i=0;i<connections_index;i++)
    {
        if(connections[i].type != ADMIN)
        {
            sprintf(resp + strlen(resp), "Client PID[%d]\n",connections[i].pid);
        }
    }
    sprintf(log,"Trimite lista cu clientii conectati spre admin PID[%d]",conn.pid);
    write_log(log);
    write_to_client(conn.sockfd, resp);
}

//functia deconecteaza clientul selectat de administator
void disconnect_selected_client(struct connection conn,int pid)
{
    int flag = 0;
    for(int i=0;i<connections_index;i++)
    {
        if(connections[i].pid == pid)
        {
            char log[64];
            sprintf(log,"Deconectare client PID[%d] la cererea administatorului PID[%d]",pid,conn.pid);
            write_log(log);
            disconnect_client(connections[i]);
            flag = 1;
            break;
        }
    }
    if(flag) write_to_client(conn.sockfd,SUCCESS);
    else write_to_client(conn.sockfd,FAILURE);
}

//functia gestioneaza conexiunea cu un administrator
void handle_admin_connection(struct connection conn)
{
    char str[BUFFER_SIZE];
    for(;;)
    {
        read_from_client(conn.sockfd, str);
        char *s;
        
        //verific tipul de request
        if(strstr(str,VIEW_LOGS))
        {
            char log[64];
            sprintf(log, "Trimte logs spre admin PID[%d]",conn.pid);
            write_log(log);
            handle_view_logs_request(conn.sockfd);
        }else if(strstr(str, VIEW_ERRORS))
        {
            char log[64];
            sprintf(log, "Trimte errors spre admin PID[%d]",conn.pid);
            write_log(log);
            handle_view_errors_request(conn.sockfd);
        }else if(strstr(str, DISCONNECT_CLIENT))    //cerere de deconectare client
        {
            send_clients_list_to_admin(conn);

        }else if(strstr(str, SELECTED_CLIENT))  //clientul ce trebuie deconectat
        {
            int selected_pid = -1;
            sscanf(str, "%*s %d",&selected_pid);
            disconnect_selected_client(conn,selected_pid);
        }
        else if(strstr(str,CLIENT_DISCONNECTED))//verific deconectarea
        {
            char log[64];
            sprintf(log, "Adminul PID[%d] s-a deconectat",conn.pid);
            write_log(log);


            disconnect_client(conn);
            //inchidem sockedtul
            close(conn.sockfd);
            //oprim executia
            exit(EXIT_SUCCESS);
        }
    }
}