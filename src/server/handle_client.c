#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#include "handle_client.h"
#include "constants.h"
#include "log.h"
#include "error_handling.h"

// functia proceseaza cererea de schimbare a vitezei
void handle_change_speed_request(int sockfd, char *input_file, double speed_factor)
{
    char output_file[50];
    char command[COMMAND_SIZE];
    char response[150];

    // creeaza numele fisierului de output
    snprintf(output_file, sizeof(output_file), "%s_output_speed_%g.mp4", input_file, speed_factor);

    // creeaza comanda ffmpeg
    snprintf(command, sizeof(command),
             "ffmpeg -i %s -filter_complex \"[0:v]setpts=%lf*PTS[v];[0:a]atempo=%lf[a]\" -map \"[v]\" -map \"[a]\" %s",
             input_file, 1.0/speed_factor, speed_factor, output_file);

    // executa comanda
    int ret = system(command);

    if(ret == 0) {
        snprintf(response, sizeof(response), "Fisierul a fost procesat cu succes si salvat ca %s\n", output_file);
    } else {
        snprintf(response, sizeof(response), "A aparut o eroare la procesarea fisierului.\n");
    }


    FILE *send_file = fopen(output_file, "rb");
    if (send_file == NULL) {
        perror("Error opening send file");
        return EXIT_FAILURE;
    }

    // trimitem size-ul fisierului
    fseek(send_file, 0, SEEK_END);
    long file_size = ftell(send_file);
    fseek(send_file, 0, SEEK_SET);

    fprintf(stdout, "file_size: %d\n", file_size);

    if (write(sockfd, &file_size, sizeof(file_size)) < 0) {
        perror("Error sending file size");
        fclose(send_file);
        close(sockfd);
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
            ssize_t result = write(sockfd, buffer + bytes_sent, bytes_read - bytes_sent);
            if (result < 0) {
                perror("Error sending data");
                fclose(send_file);
                close(sockfd);
                return EXIT_FAILURE;
            }
            bytes_sent += result;
        }
    }
}

// functia proceseaza cererea de schimbare a vitezei
void handle_change_speed_request_streaming(int sockfd, char *input_file, double speed_factor)
{
    char output_file[50];
    char command[COMMAND_SIZE];
    char response[150];

    // creeaza numele fisierului de output
    snprintf(output_file, sizeof(output_file), "%s_output_speed_%g.mp4", input_file, speed_factor);

    // creeaza comanda ffmpeg
    snprintf(command, sizeof(command),
             "ffmpeg -i %s -filter_complex \"[0:v]setpts=%lf*PTS[v];[0:a]atempo=%lf[a]\" -map \"[v]\" -map \"[a]\" %s",
             input_file, 1.0/speed_factor, speed_factor, output_file);

    // executa comanda
    int ret = system(command);


    FILE *fp;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    // obtinem portul socket-ului client
    if (getsockname(sockfd, (struct sockaddr*)&addr, &addr_len) == -1) {
        perror("getsockname");
        return 1;
    }

    int port = ntohs(addr.sin_port);

    // rulam comanda ffmpeg pentru a trimite video-ul catre client prin streaming
    snprintf(command, sizeof(command), "ffmpeg -re -i %s -f mpegts udp://127.0.0.1:%s", output_file, "1234");
    system(command);
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

//functia gestioneaza conexiunea cu un client
void handle_client_connection(struct connection conn)
{
    char str[BUFFER_SIZE];
    for(;;)
    { 
        read_from_client(conn.sockfd, str);

        int response = 1;
        write(conn.sockfd, &response, sizeof(response));

        char *s;
        //do something with received data
        char log[64];
        sprintf(log, "%s", str);
        write_log(log);
        
        //check for disconnect
        if(strstr(str,CLIENT_DISCONNECTED))
        {
            char log[64];
            sprintf(log, "Clientul PID[%d] s-a deconectat", conn.pid);
            write_log(log);
            disconnect_client(conn);
            //inchidem sockedtul
            close(conn.sockfd);
            //oprim executia
            exit(EXIT_SUCCESS);
        }
        if(strstr(str, "CHANGE_SPEED"))
        {
            char input_file[BUFFER_SIZE], mode[BUFFER_SIZE];
            double speed_factor;
            char log[64];

            // primire size fisier
            long file_size;
            if (recv(conn.sockfd, &file_size, sizeof(file_size), 0) < 0) {
                perror("recv size failed");
                exit(EXIT_FAILURE);
            }

            sprintf(log, "%d\n", file_size);
			write_log(log);

            size_t total_received = 0;
            char buffer[1024];

            time_t current_time;
            struct tm *local_time;
            char filename[100];

            current_time = time(NULL);
            local_time = localtime(&current_time);
            strftime(filename, sizeof(filename), "%Y%m%d%H%M%S_file.mp4", local_time);

            FILE* output_file = get_file(filename);

            fprintf(stdout, "Receiving file...\n");
            while (total_received < file_size) {
                size_t chunk_size = 1024;
                if (file_size - total_received < chunk_size) {
                    chunk_size = file_size - total_received; 
                }

                ssize_t bytes_received = recv(conn.sockfd, buffer, chunk_size, 0);
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

            // parsez request-ul
            sscanf(str, "CHANGE_SPEED %s %s %lf", mode, filename, &speed_factor);
            if (strcmp(mode, "DOWNLOAD") == 0)
            {
                // procesez request-ul
                handle_change_speed_request(conn.sockfd, filename, speed_factor);
            } 
            else if (strcmp(mode, "STREAMING") == 0) {
                handle_change_speed_request_streaming(conn.sockfd, filename, speed_factor);
            }
        }

        //send the response to server
        write_to_client(conn.sockfd, str);
    }
}

