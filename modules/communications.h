#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <mqueue.h>

#define NEW_SON "[NEW_SON]"
#define CONOK "[CONOK]"
#define CONKO "[CONKO]"
#define LIST_REQUEST "[LIST_REQUEST]"
#define LIST_RESPONSE "[LIST_RESPONSE]"
#define SEND_MSG "[MSG]"
#define MSGOK "[MSGOK]"
#define SEND_FILE "[NEW_FILE]"
#define FILE_DATA "[FILE_DATA]"
#define CHECK_OK "[CHECK_OK]"
#define CHECK_KO "[CHECK_KO]"
#define EXIT "[EXIT]"
#define UNKNOWN "[UNKNOWN]"
#define NEW_MSG "[NEW_MSG]"
#define DEFAULT_MSG_SIZE 512
#define MAX_PACKET_SIZE 498 //512-14

typedef struct {
    uint8_t type;
    char *header;
    uint16_t length;
    char *data;
} Packet;


//Writes a packet to the specified file descriptor
int COM_writePacket(char *header, unsigned short length, char *data, int exit, int ipc, int fd);

//Reads a packet from the specified file descriptor
Packet * COM_readPacket(int ipc, int fd);

//Frees a packet
void COM_freePacket(Packet *packet);

int COM_isConnectionPacket(Packet *packet);

char * COM_serializePacket(Packet *p, size_t *packetSize);
Packet * COM_deserializePacket(const char *data);

int COM_packetsRequired(int fileSize);

#endif
