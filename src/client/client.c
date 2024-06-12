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
    int send_option;

    char str[BUFFER_SIZE];

    // calea catre fisier
    fprintf(stdout, "Introduceți calea fișierului de intrare: ");
    fscanf(stdin, "%s", input_file);

    fprintf(stdout, "Introduceți factorul de viteză (de exemplu, 2.0 pentru dublarea vitezei): ");
    fscanf(stdin, "%lf", &speed_factor);

    fprintf(stdout, "Cum doresti sa fie transmis video-ul procesat\n1. Descarcare\n2. Streaming\n");
    fscanf(stdin, "%d", &send_option);

    fprintf(stdout, "Procesarea fisierului video a inceput. Asteptati...\n");

    if (send_option == 1)
    {
        // request-ul catre server
        snprintf(str, sizeof(str), "CHANGE_SPEED DOWNLOAD %s %lf", input_file, speed_factor);

        // trimite request-ul catre server
        write_to_server(str);

        // citeste raspunsul de la server
        read_from_server(str);

        // afiseaza raspunsul
        fprintf(stdout, "%s\n", str);
    } else if (send_option == 2) {
        // request-ul catre server
        snprintf(str, sizeof(str), "CHANGE_SPEED STREAMING %s %lf", input_file, speed_factor);
        write_to_server(str);

        int sockfd;
        struct sockaddr_in server_addr;
        char buffer[65536];

        // creez socket-ul UDP
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        memset(&server_addr, 0, sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(INADDR_ANY);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("bind");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // deschid un pipe care trimite output-ul de la server catre ffplay
        FILE *pipe = popen("ffplay -i -", "w");
        if (!pipe) {
            perror("popen");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        while (1) {
            int n;
            socklen_t len = sizeof(server_addr);

            // astept input de la server
            n = recvfrom(sockfd, (char *)buffer, 65536, MSG_WAITALL,
                        (struct sockaddr *)&server_addr, &len);
            if (n < 0) {
                perror("recvfrom");
                pclose(pipe);
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            // scriu output-ul de la server in pipe
            fwrite(buffer, 1, n, pipe);
            fflush(pipe);
        }   
    } else {
        fprintf(stdout, "Optiune necunoscuta!");
    }
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
    fprintf(stdout,"1.Scrie un mesaj spre server\n");
    fprintf(stdout,"2. Schimba viteza unui fisier video\n");
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

void handle_option(int option)
{
    switch(option)
    {
        case 1:
            handle_write_option();
            break;
        case 2:
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
        else if(strstr(buf, "2")) option = 2;
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
