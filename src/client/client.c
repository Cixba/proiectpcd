#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

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

void send_file_to_server(const char* send_file_path, int sock)
{
    // deschidem fisierul ce trebuie trimis
    FILE *send_file = fopen(send_file_path, "rb");
    if (send_file == NULL) {
        perror("Error opening send file");
        return EXIT_FAILURE;
    }

    // trimitem size-ul fisierului
    fseek(send_file, 0, SEEK_END);
    long file_size = ftell(send_file);
    fseek(send_file, 0, SEEK_SET);

    if (write(sock, &file_size, sizeof(file_size)) < 0) {
        perror("Error sending file size");
        fclose(send_file);
        close(sock);
        return EXIT_FAILURE;
    }

    // trimitem fisierul byte cu byte catre server
    char buffer[1024];
    size_t bytes_read;

    // Read the file in chunks and send each chunk
    fprintf(stdout, "Sending file\n");
    while ((bytes_read = fread(buffer, 1, 1024, send_file)) > 0) {
        size_t bytes_sent = 0;
        while (bytes_sent < bytes_read) {
            ssize_t result = write(sock, buffer + bytes_sent, bytes_read - bytes_sent);
            if (result < 0) {
                perror("Error sending data");
                fclose(send_file);
                close(sock);
                return EXIT_FAILURE;
            }
            bytes_sent += result;
        }
    }

    fprintf(stdout, "Sent file\n");
}

FILE* get_file(char* filename) {
    FILE *fp;

    // Open the file for writing
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Error opening file");
        return NULL;
    }

    return fp;
}

// functia schimba viteza unui fisier video
void handle_change_speed_option()
{
    char input_file[BUFFER_SIZE];
    double speed_factor;
    int send_option;
    char str[BUFFER_SIZE];

    // calea catre fisier
    fprintf(stdout, "Introduceti calea fisierului de intrare: ");
    fscanf(stdin, "%s", input_file);

    fprintf(stdout, "Introduceti factorul de viteza (de exemplu, 2.0 pentru dublarea vitezei): ");
    fscanf(stdin, "%lf", &speed_factor);

    fprintf(stdout, "Cum doresti sa fie transmis video-ul procesat\n1. Descarcare\n2. Streaming\n");
    fscanf(stdin, "%d", &send_option);

    fprintf(stdout, "Procesarea fisierului video a inceput. Asteptati...\n");
    fprintf(stdout, "%d", send_option);

    if (send_option == 1)
    {
        // request-ul catre server
        snprintf(str, sizeof(str), "CHANGE_SPEED DOWNLOAD %s %lf", input_file, speed_factor);
        write_to_server(str);

        int response;
        if (recv(sockfd, &response, sizeof(response), 0) < 0) {
            perror("recv size failed");
            exit(EXIT_FAILURE);
        }

        send_file_to_server(input_file, sockfd);

        long file_size;
        if (recv(sockfd, &file_size, sizeof(file_size), 0) < 0) {
            perror("recv size failed");
            exit(EXIT_FAILURE);
        }

        size_t total_received = 0;
        char buffer[1024];

        time_t current_time;
        struct tm *local_time;
        char filename[100];

        current_time = time(NULL);
        local_time = localtime(&current_time);
        strftime(filename, sizeof(filename), "%Y%m%d%H%M%S_result_file.mp4", local_time);

        FILE* output_file = get_file(filename);

        fprintf(stdout, "Receiving file...\n");
        while (total_received < file_size) {
            size_t chunk_size = 1024;
            if (file_size - total_received < chunk_size) {
                chunk_size = file_size - total_received; 
            }

            ssize_t bytes_received = recv(sockfd, buffer, chunk_size, 0);
            if (bytes_received < 0) {
                perror("recv data failed");
                fclose(output_file);
                exit(EXIT_FAILURE);
            }

            fwrite(buffer, 1, bytes_received, output_file);
            fflush(output_file);
            total_received += bytes_received;
        }
        fprintf(stdout, "Received file\n");
    } else if (send_option == 2) {
        // request-ul catre server
        snprintf(str, sizeof(str), "CHANGE_SPEED STREAMING %s %lf", input_file, speed_factor);
        write_to_server(str);

        int response;
        if (recv(sockfd, &response, sizeof(response), 0) < 0) {
            perror("recv size failed");
            exit(EXIT_FAILURE);
        }

        send_file_to_server(input_file, sockfd);

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
        server_addr.sin_port = htons(1234);
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

