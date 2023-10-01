#ifndef EASYSTEM_STDINOUT_H
#define EASYSTEM_STDINOUT_H

#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <pthread.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define COLOR_BOLD  "\e[1m"
#define RESET "\x1B[0m"

void INOUT_readStdin(char* *formattedBuff,char* *buff);
void INOUT_print(char* buff);
void INOUT_end();

#endif //EASYSTEM_STDINOUT_H
