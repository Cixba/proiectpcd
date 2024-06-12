#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#include "constants.h"
#include "error_handling.h"

enum {
    WRITE_MESSAGE,
    CHANGE_SPEED
};

int request;
int sockfd;

void write_to_server(char *str);
void read_from_server(char *str);

int create_socket(struct sockaddr_in *serv_addr)
{
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) handle_tcp_socket_error(errno);

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(PORT);
    serv_addr->sin_addr.s_addr = inet_addr(ADDR);

    return sockfd;
}

// functia schimba viteza unui fisier video
void handle_change_speed_option()
{
    char input_file[BUFFER_SIZE];
    double speed_factor;
    char str[BUFFER_SIZE];

    // calea catre fisier
    fprintf(stdout, "Introduceti calea fisierului de intrare: ");
    fscanf(stdin, "%s", input_file);

    fprintf(stdout, "Introduceti factorul de viteza (de exemplu, 2.0 pentru dublarea vitezei): ");
    fscanf(stdin, "%lf", &speed_factor);

    fprintf(stdout, "Procesarea fisierului video a inceput. Asteptati...\n");

    // request-ul catre server
    snprintf(str, sizeof(str), "CHANGE_SPEED %s %lf", input_file, speed_factor);

    // trimite request-ul catre server
    write_to_server(str);

    // citeste raspunsul de la server
    read_from_server(str);

    // afiseaza raspunsul
    fprintf(stdout, "%s\n", str);
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
}

//functia afiseaza menuil
void make_menu()
{
    fprintf(stdout,"Alege o optiune:\n");
    fprintf(stdout,"1. Schimba viteza unui fisier video\n");
}

void handle_option(int option)
{
    switch(option)
    {
        case 1:
            handle_change_speed_option();
            break;
        default:
            fprintf(stdout, "Optiune invalida");
    }
}

void ui()
{
    int option;
    char buf[50];

    while(1)
    {
        fprintf(stdout,"\n");
        make_menu();
        fprintf(stdout,"\n");
        
        fprintf(stdout,"<<< ");
        //fscanf(stdin, "%d",&option);
        fscanf(stdin, "%s%*[^\n]%*c", buf);
        if(strstr(buf, "1")) option = 1;
        else option = -1;

        handle_option(option);
    }
}

//sigint, sigtstp handler
void handle_sigint_sigtstp(int sig)
{
    write_to_server(CLIENT_DISCONNECTED);
    fprintf(stdout,"\n");
    exit(EXIT_SUCCESS);
}

void handle_sigusr1(int sig)
{
    fprintf(stdout, "Ai fost deconectat de catre un administrator\n");
    exit(EXIT_FAILURE);
}

//handler pentru sigterm
//va indica faptul ca serverul este deconectat
void handle_sigterm(int sig)
{
    fprintf(stdout,"\nServerul a fost oprit\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    struct sockaddr_in serv_addr;
    char buffer[10];

    signal(SIGINT, handle_sigint_sigtstp);
    signal(SIGTSTP, handle_sigint_sigtstp);
    signal(SIGTERM, handle_sigterm);
    signal(SIGUSR1, handle_sigusr1);

    sockfd = create_socket(&serv_addr);

    if ((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) handle_tcp_connect_error(errno);

    //trimit pid ul clientului catre server
    sprintf(buffer, "%d",(int)getpid());
    if(write(sockfd, buffer, strlen(buffer)) < 0) handle_write_error(errno);

    ui();

    return 0;
}