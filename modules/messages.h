#ifndef EASYSTEM_MESSAGES_H
#define EASYSTEM_MESSAGES_H

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>

typedef struct {
    char *originUser;
    char *message;
}Message;

typedef struct {
    char *originUser;
    char *fileName;
    char *hashDM5;
    char *fileSize;
}File;

/**
 * Parse the msg and name for send
 * @param buff The name and msg to read
 * @return The name and msg if send okey. Or Null if not send good
 */
void * FP_parseMsgFile(char *buff);

//Converts data received to a Message struct
Message * MS_deserializeMessage(const char *data);
char* MS_serializeMessage(Message *message);
void MS_freeMessage(Message* message);
char* MS_serializeFile(File *file);
File* MS_deserializeFile(const char *data);
void MS_freeFile(File* file);


#endif
