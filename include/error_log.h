#ifndef ERROR_LOG_H
#define ERROR_LOG_H

//int errfd;

int open_error_file();
char * get_local_time_error();
int write_error(char *str);

#endif