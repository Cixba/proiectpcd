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
    char output_file[50];
    char command[COMMAND_SIZE];
    char response[150];

    // creeaza numele fisierului de output
    snprintf(output_file, sizeof(output_file), "output_speed_%g.mp4", speed_factor);

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

    // trimite raspunsul catre client
    write_to_client(sockfd, response);
}

//functia gestioneaza conexiunea cu un client
void handle_client_connection(struct connection conn)
{
    char str[BUFFER_SIZE];
    for(;;)
    { 
        read_from_client(conn.sockfd, str);
        char *s;
        //do something with received data
        
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
            char input_file[100];
            double speed_factor;
            char log[150];

            // parsez request-ul
            sscanf(str, "CHANGE_SPEED %s %lf", input_file, &speed_factor);

            // procesez request-ul
            handle_change_speed_request(conn.sockfd, input_file, speed_factor);

            // scriu in fisierul de log
            sprintf(log, "[%s] Schimbare viteza pentru fisierul %s cu factorul %lf\n", get_local_time(), input_file, speed_factor);
            write_log(log);
        }
        
    }
    
}