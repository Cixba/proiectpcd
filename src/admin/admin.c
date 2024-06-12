#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <signal.h>

#include "constants.h"
#include "error_handling.h"

int sockfd;

void read_multiple_data();

int create_socket(struct sockaddr_un *serv_addr)
{
    int sockfd;
    //struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) handle_tcp_socket_error(errno);

    serv_addr->sun_family = AF_UNIX;
    strcpy(serv_addr->sun_path, UNIX_SOCKET_PATH);

    return sockfd;
}

//functia scrie catre server continutul din str
void write_to_server(char *str)
{
    if(strlen(str) > 0) 
    {
        if(write(sockfd, str, strlen(str)) < 0) handle_write_error(errno);
    }
}

//functia citeste de la server si pune datele in str
void read_from_server(char *str)
{
    if(read(sockfd, str, BUFFER_SIZE) < 0) handle_read_error(errno);

    //verific daca serverul s-a oprit
    if(strstr(str, "server_down"))
    {
        fprintf(stdout, "Serverul este oprit\n");
        exit(EXIT_FAILURE);
    }
}

void make_menu()
{
    fprintf(stdout,"Alege o optiune:\n");
    fprintf(stdout,"1.Vizualizare erori\n");
    fprintf(stdout,"2.Vizualizare log-uri\n");
    fprintf(stdout,"3.Deconecteaza un client");
}

void handle_disconnect_client_request()
{
    char str[BUFFER_SIZE];
    int selected_pid;
    //trimit mesajul de request spre server
    write_to_server(DISCONNECT_CLIENT);

    //astept lista cu clientii conecati de la server
    read_from_server(str);
    fprintf(stdout, "%s", str);
    fprintf(stdout, "Alege un client(pid): ");
    fscanf(stdin,"%d",&selected_pid);

    //trimit clientul selectat spre server
    sprintf(str, "%s %d",SELECTED_CLIENT,selected_pid);
    write_to_server(str);

    read_from_server(str);

    if(strstr(str, SUCCESS)) fprintf(stdout, "Client deconectat cu succes\n");
    else fprintf(stdout, "Clientul nu a putut fi deconectat\n");
}

//functie exemplu de trimitere mesaj spre server
void handle_write_option()
{
    char str[BUFFER_SIZE];
    //input client
    fprintf(stdout, "Scrie un mesaj: ");
    fscanf(stdin,"%*[\n]");         //citesc un \n
    fscanf(stdin,"%[^\n]%*c",str);  //citesc tot mai putin \n in str, apoi citesc si \n
    
    //trimit mesaj spre server
    write_to_server(str);

    //astept raspunsul de la verver
    read_from_server(str);
    
    //afisez raspunsul clientului
    fprintf(stdout,"%s",str);
}

void handle_view_errors()
{
    //trimit mesajul de request spre server
    write_to_server(VIEW_ERRORS);

    //astept raspunsul de la server
    read_multiple_data();
}

void read_multiple_data()
{
    char str[BUFFER_SIZE];
    int keep_going = 1;
    int bytes_read;
    while(keep_going && (bytes_read = read(sockfd, str, BUFFER_SIZE)) > 0)
    {
        if(bytes_read < strlen(str)) str[bytes_read] = '\0';

        if(strstr(str, EOM))
        {
            str[strlen(str) - strlen(EOM)] = '\0';
            keep_going = 0;
        }
        fprintf(stdout,"%s",str);
    }
}

void handle_view_logs()
{
    fprintf(stdout, "trimit view logs spre server\n");
    //trimit mesajul de request spre server
    write_to_server(VIEW_LOGS);

    char logs_list[BUFFER_SIZE];
    char log_file[100];
    //astept lista de logs
    read_from_server(logs_list);
    strcpy(log_file, "empty");
    //verific alegerea
    while(!strstr(logs_list, log_file))
    {
        fprintf(stdout, "Logs:\n%s\n", logs_list);
        fprintf(stdout, "Alege un fisier log: ");
        log_file[0] = '\0';
        fscanf(stdin, "%s",log_file);
    }
    //trimit alegerea spre server
    write_to_server(log_file);

    //astept raspunsul de la server

    read_multiple_data();
}

void handle_option(int option)
{
    switch(option)
    {
        case 1:
            handle_view_errors();
            break;
        case 2://vizualizare logs
            handle_view_logs();
            break;
        case 3://deconectare client
            handle_disconnect_client_request();
            break;
        default:
            fprintf(stdout, "Optiune invalida");
    }
}

//sigint handler
void handle_sigint_sigtstp(int sig)
{
    write_to_server(CLIENT_DISCONNECTED);
    fprintf(stdout,"\n");
    exit(EXIT_SUCCESS);
}

//handler pentru sigterm
//va indica faptul ca serverul este deconectat
void handle_sigterm(int sig)
{
    fprintf(stdout,"\nServerul a fost oprit\n");
    exit(EXIT_FAILURE);
}

void ui()
{
    int option, i=0;
    char buf[50];

    while(1)
    {
        fprintf(stdout,"\n");
        make_menu();
        fprintf(stdout,"\n");
        
        fprintf(stdout,"<<< ");
        //fscanf(stdin, "%d",&option);
        fscanf(stdin, "%s%*[^\n]%*c", buf);
        fprintf(stdout,"after read");

        //verific optiunea introdusa
        if(strstr(buf, "1")) option = 1;
        else if(strstr(buf,"2")) option = 2;
        else if(strstr(buf,"3")) option = 3;
        else option = -1;

        handle_option(option);
    }
}

int main(int argc, char* argv[]) {
    struct sockaddr_un serv_addr;
    char buffer[10];

    signal(SIGINT, handle_sigint_sigtstp);
    signal(SIGTSTP, handle_sigint_sigtstp);
    signal(SIGTERM, handle_sigterm);

    sockfd = create_socket(&serv_addr);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) handle_unix_connect_error(errno);

    //trimit pid ul clientului catre server
    sprintf(buffer, "%d",(int)getpid());
    if(write(sockfd, buffer, strlen(buffer)) < 0) handle_write_error(errno);
    ui();

    return 0;
}

