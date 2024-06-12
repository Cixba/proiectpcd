#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "handle_client.h"
#include "constants.h"
#include "log.h"
#include "error_handling.h"

// functia proceseaza cererea de schimbare a vitezei
void handle_change_speed_request(int sockfd, char *input_file, double speed_factor)
{
    char output_file[BUFFER_SIZE];
    char command[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // creeaza numele fisierului de output
    snprintf(output_file, sizeof(output_file), "output_speed_%g.mp4", speed_factor);

    // creeaza comanda ffmpeg
    snprintf(command, sizeof(command),
             "ffmpeg -i %s -filter_complex \"[0:v]setpts=%lf*PTS[v];[0:a]atempo=%lf[a]\" -map \"[v]\" -map \"[a]\" %s",
             input_file, 1.0/speed_factor, speed_factor, output_file);

    // executa comanda
    int ret = system(command);

    if(ret == 0) {
        snprintf(response, sizeof(response), "Fișierul a fost procesat cu succes și salvat ca %s\n", output_file);
    } else {
        snprintf(response, sizeof(response), "A apărut o eroare la procesarea fișierului.\n");
    }

    // trimite raspunsul catre client
    write_to_client(sockfd, response);
}

// functia proceseaza cererea de schimbare a vitezei
void handle_change_speed_request_streaming(int sockfd, char *input_file, double speed_factor)
{
    FILE *fp;
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];
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
    snprintf(command, sizeof(command), "ffmpeg -re -i %s -f mpegts udp://127.0.0.1:%s", input_file, port);

    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        return;
    }
}


//functia gestioneaza conexiunea cu un client
void handle_client_connection(struct connection conn)
{
    printf("Hm");
    char str[BUFFER_SIZE];
    for(;;)
    { 
        read_from_client(conn.sockfd, str);
        char *s;
        
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

            // parsez request-ul
            sscanf(str, "CHANGE_SPEED %s %s %lf", mode, input_file, &speed_factor);
            if (strcmp(mode, "DOWNLOAD") == 0)
            {
                // procesez request-ul
                handle_change_speed_request(conn.sockfd, input_file, speed_factor);
            } else if (strcmp(mode, "STREAMING") == 0) {
                handle_change_speed_request_streaming(conn.sockfd, input_file, speed_factor);
            }

            // scriu in fisierul de log
            sprintf(log, "[%s] Schimbare viteza pentru fisierul %s cu factorul %lf\n", get_local_time(), input_file, speed_factor);
            write_log(log);
        }

        //send the response to server
        write_to_client(conn.sockfd, str);
    }
    
}