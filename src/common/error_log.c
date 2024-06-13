#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>       //flock
#include <time.h>           //time, localtime

#include "error_handling.h"
#include "error_log.h"
#include "constants.h"

int errfd = -1;

//deschide fisierul de erori          
int open_error_file()
{
    //deschid fisierul daca exista
    /*if((errfd = open(ERROR_PATH, O_WRONLY)) < 0)
    {
        if((errfd = open(ERROR_PATH, O_WRONLY | O_CREAT, 0666)) < 0) handle_open_error(errno);
    }*/
    if((errfd = open(ERROR_PATH, O_WRONLY | O_CREAT, 0666)) < 0) handle_open_error(errno);
    
    return errfd;
}

//timpul curent
char * get_local_time_error()
{
    time_t logoff_time;
    struct tm *local_time;
    char *s;
    time(&logoff_time);
    local_time = localtime(&logoff_time);

    s = asctime(local_time);
    s[strlen(s)-1] = (char) 0;  //sterge \n

    return s;
}

//scriere mesaj eroare
int write_error(char *str)
{
    if(errfd == -1) errfd = open_error_file();
    char log[1024];
    if(strlen(str) == 0) return 0;
    sprintf(log, "[%s] %s\n",get_local_time_error(), str);
    flock(errfd,LOCK_EX);
    if(write(errfd, log, strlen(log)) < 0) handle_write_error(errno);
    flock(errfd,LOCK_UN);
    return 1;
}
