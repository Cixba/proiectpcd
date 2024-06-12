#ifndef LOG_H
#define LOG_H

extern char log_file_name[];

int write_log(char *log);
int open_log_file();
char * get_local_time();

#endif