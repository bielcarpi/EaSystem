#ifndef EASYSTEM_ILUVATARTHREADS_H
#define EASYSTEM_ILUVATARTHREADS_H

#define _GNU_SOURCE
#include <pthread.h>
#include <sys/file.h>
#include <signal.h>
#include "fileparser.h"
#include "communications.h"
#include "messages.h"
#include "ipcHelper.h"
#include "socketHelper.h"

#define PS2 YEL COLOR_BOLD"$ "
#define ERR_FILE_OPEN RED"\nError. This file is blocked as it's being read or written in this moment.\n\n"RESET
#define MSG_CORRECTLY_SENT "\nMessage correctly sent.\n\n"
#define ERR_SENDING_MSG RED"\nUnexpected error sending the message. Please, try again.\n\n"
#define FILE_CORRECTLY_SENT GRN"\nFile %s correctly sent to %s.\n\n"
#define FILE_INCORRECTLY_SENT RED"\nThe file %s was sent to %s, but MD5SUMs don't match.\n\n"
#define FILE_CORRECTLY_RECEIVED GRN"\nThe file %s from %s was received correctly. Now you have it on your folder.\n\n"
#define FILE_INCORRECTLY_RECEIVED RED"\nThe file %s from %s was received, but MD5SUMs don't match.\n\n"
#define ERR_WRITING_FILE RED"\nError writing the file. Aborting...\n\n"
#define ERR_CONN_ILUVATAR RED"\nError. Can't connect to the desired Iluvatar Son.\n\n"
#define ERR_FILE_INVALID RED"\nError. The file is not valid. Did you specify a user and write the file correctly?\n" RESET "Example: send file Galadriel image.png\n\n"
#define ERR_FILE_EXTENSION RED"\nError. The file introduced has not a valid extension, and it can't be sent.\n\n"
#define ERR_FILE_EXIST RED"\nError. The file introduced does not exist.\n\n"
#define ERR_SENDING_FILE RED"\nUnexpected error sending the file. Please, try again.\n\n"
#define SENDING_FILE "\nSending file %s to %s ...\n\n"
#define RECEIVED_MSG "\nNew Message Received!" BLU COLOR_BOLD " %s " RESET ", from %s (%s) sent: " CYN "%s\n\n"
#define RECEIVED_MSG_NEIGHBOUR "\nNew Message Received! The neighbour" BLU COLOR_BOLD " %s " RESET "sent: " CYN "%s\n\n"
#define RECEIVED_FILE "\nDownloading New File!" BLU COLOR_BOLD " %s " RESET ", from %s (%s) sent: %s\nSaving into " CYN "%s/%s\n\n"
#define RECEIVED_FILE_NEIGHBOUR "\nDownloading New File! The neighbour" BLU COLOR_BOLD " %s " RESET  "sent: %s\nSaving into " CYN "%s/%s\n\n"
#define UNKNOWN_PACKET_ILUVATAR RED"\nUnknown Iluvatar packet received, with header %s\n\n"
#define UNKNOWN_PACKET_IPC RED"\nUnknown IPC packet received, with header %s\n\n"
#define STOP_RECEIVING_FILE RED"\nStopped receiving the file %s from %s.\n\n"
#define STOP_SENDING_FILE RED"\nStopped sending file %s to %s.\n\n"
#define END_RECEIVING_FILE RED"\n%s disconnected and stopped receiving the file %s.\n\n"
#define UNUSED(x) (void)(x)

typedef struct{
    pthread_t threadId;
    File *f;
    Message *m;
    Connection *c;
    Packet *p;
    IluvatarSonConfig *config;
    int fd;
    int ipc;
    char *idIpc;
    int *run;
    pthread_rwlock_t *mtxRun;
    pthread_mutex_t *mtxFdArda;
} ThreadArgs;

int ITH_threadArgsCompareIDs(const void *a, const void *b, void *udata);
uint64_t ITH_threadArgsHashByIDs(const void *item, uint64_t seed0, uint64_t seed1);
void ITH_getThreadArgsList(struct hashmap *map, int *numThreadArgs, ThreadArgs ***threadArgs);

void* ITH_sendMessage(void *args);
void* ITH_sendFile(void *args);
void* ITH_checkSocketMessage(void *args);
void* ITH_checkIpcMessage(void *args);
void ITH_freeThreadArgs(ThreadArgs *args);

#endif
