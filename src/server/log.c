
#include <stdio.h>          //sprintf
#include <time.h>           //time, localtime
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>       //flock
#include <time.h>

#include "log.h"
#include "error_handling.h"
#include "constants.h"

int logfd;

char log_file_name[128];

char * get_local_time()
{
    time_t logoff_time;
    struct tm *local_time;
    char *s;
    time(&logoff_time);
    char log[64];
    local_time = localtime(&logoff_time);

    s = asctime(local_time);
    s[strlen(s)-1] = (char) 0;  //sterge \n

    return s;
}

//deschide fisierul de logs
int open_log_file()
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    strftime(log_file_name, sizeof(log_file_name)-1, "log_%Y%m%d_%H%M%S.txt", t);
    
    if ((logfd = open(log_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) handle_open_error(errno);
}

int write_log(char *str)
{
    char log[LOG_SIZE];
    if(strlen(str) == 0) return 0;
    sprintf(log, "[%s] %s\n",get_local_time(), str);
    flock(logfd,LOCK_EX);
    if(write(logfd, log, strlen(log)) < 0) handle_write_error(errno);
    flock(logfd,LOCK_UN);
    return 1;
}