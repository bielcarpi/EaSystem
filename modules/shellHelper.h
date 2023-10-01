#ifndef SHELLHELPER_H
#define SHELLHELPER_H

#define _GNU_SOURCE
#include <sys/wait.h>
#include "stdinout.h"

#define ERR_UNKNOWN_COMMAND "Unknown Command\n\n"
#define ERR_COMMAND "Command Error"
#define ERR_FORK "\x1B[31mError. Can't create a fork\n\x1B[0m"

void SHH_runCommand(char *command);
char * SHH_getDomain(char *ip);
char * SHH_getHashMD5(char* fileName);
int SHH_processExists(int pid);
char * SHH_getBasename(char *filePath);

#endif