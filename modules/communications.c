#include "communications.h"

static char getType(char *header, int exit){
    if(!exit){
        if(strcmp(header, NEW_SON) == 0 || strcmp(header, CONOK) == 0 || strcmp(header, CONKO) == 0)
            return 1;
        if(strcmp(header, LIST_REQUEST) == 0 || strcmp(header, LIST_RESPONSE) == 0)
            return 2;
        else if(strcmp(header, SEND_MSG) == 0 || strcmp(header, MSGOK) == 0)
            return 3;
        else if(strcmp(header, SEND_FILE) == 0 || strcmp(header, FILE_DATA) == 0)
            return 4;
        else if(strcmp(header, CHECK_OK) == 0 || strcmp(header, CHECK_KO) == 0)
            return 5;
        else if(strcmp(header, NEW_MSG) == 0)
            return 8;
    }
    else{
        if(strcmp(header, EXIT) == 0 || strcmp(header, CONOK) == 0 || strcmp(header, CONKO) == 0)
            return 6;
    }

    //If none of the above, return UNKNOWN
    return 7;
}


static int isPacketValid(Packet *p){
    //We'll check the type of the packet and whether its contents are OK nor not
    //If not, we'll return an error
    if(p == NULL) return 0;

    if(p->type <= 0 || p->type >= 9){
        return 0;
    }

    if(p->type == 1){
        if(strcmp(p->header, NEW_SON) != 0 && strcmp(p->header, CONOK) != 0 && strcmp(p->header, CONKO) != 0){
            return 0;
        }
    }else if(p->type == 2){
        if(strcmp(p->header, LIST_REQUEST) != 0 && strcmp(p->header, LIST_RESPONSE) != 0){
            return 0;
        }
    }else if(p->type == 3){
        if(strcmp(p->header, SEND_MSG) != 0 && strcmp(p->header, MSGOK) != 0){
            return 0;
        }
    }else if(p->type == 4){
        if(strcmp(p->header, SEND_FILE) != 0 && strcmp(p->header, FILE_DATA) != 0){
            return 0;
        }
    }else if(p->type == 5){
        if(strcmp(p->header, CHECK_OK) != 0 && strcmp(p->header, CHECK_KO) != 0){
            return 0;
        }
    }else if(p->type == 6){
        if(strcmp(p->header, EXIT) != 0 && strcmp(p->header, CONOK) != 0 && strcmp(p->header, CONKO) != 0){
            return 0;
        }
    }else if(p->type == 7){
        if(strcmp(p->header, UNKNOWN) != 0){
            return 0;
        }
    }else if(p->type == 8){
        if(strcmp(p->header, NEW_MSG) != 0){
            return 0;
        }
    }


    return 1;
}

static char * readUntil(int fd, char cFi) {
    int i = 0;
    char c = '0';
    char* buffer = (char*)malloc(sizeof(char));

    while (c != cFi) {
        read(fd, &c, sizeof(char));
        if(i == 0 && c == '0'){ //If the first thing received is a '0', implies end of connection
            free(buffer);
            return NULL;
        }
        if (c != cFi) {
            buffer[i] = c;
            buffer = (char*)realloc(buffer, sizeof(char) * (i + 2));
        }
        i++;
    }

    buffer[i - 1] = '\0';
    return buffer;
}


int COM_isConnectionPacket(Packet *p){
    if(strcmp(p->header, NEW_SON) == 0) return 1;
    return 0;
}

int COM_writePacket(char *header, unsigned short length, char *data, int exit, int ipc, int fd){
    Packet *p = (Packet *) malloc(sizeof(Packet));
    memset(p, 0, sizeof(Packet));

    p->type = getType(header, exit);
    if(p->type == 7){
        p->header = UNKNOWN;
    }
    else{
        p->header = header;
        p->length = length;
        p->data = data;
    }

    //Check if packet is valid. If not, return error
    if(!isPacketValid(p)) {
        free(p);
        return 0;
    }

    char *buff;
    size_t size;
    buff = COM_serializePacket(p, &size);
    free(p);
    if(buff == NULL) return -1;

    int err;
    if(ipc){
        buff = (char *) realloc(buff, DEFAULT_MSG_SIZE);
        memset(&buff[size], 0, DEFAULT_MSG_SIZE-size);

        struct timespec tm;
        clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += 5; // Set for 5 seconds
        err = mq_timedsend(fd, buff, DEFAULT_MSG_SIZE, 0, &tm);
    }
    else err = (int) write(fd, buff, size);

    free(buff);
    return err;
}


Packet * COM_readPacket(int ipc, int fd){
    char *buff;

    if(ipc){
        buff = (char *) malloc(DEFAULT_MSG_SIZE);
        memset(buff, 0, DEFAULT_MSG_SIZE);
        struct timespec tm;
        clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += 5; // Set for 5 seconds
        int err = mq_timedreceive(fd, buff, DEFAULT_MSG_SIZE, NULL, &tm);
        if(err <= 0){
            free(buff);
            return NULL;
        }
        char filename[] = "tmpXXXXXX";
        fd = mkstemp(filename);
        unlink(filename); //File will be deleted when fd is closed

        for(int i = 0; i < DEFAULT_MSG_SIZE; i++) //Write contents of buff to the file
            write(fd, &buff[i], sizeof(char));

        lseek(fd, 0, SEEK_SET); //Reposition file cursor to initial position, to start reading
        free(buff);
    }

    buff = readUntil(fd, ']');
    if(buff == NULL){
        if(ipc) close(fd);
        return NULL;
    }

    size_t size = strlen(buff);
    buff = (char *) realloc(buff, size + 3); //1 for ']' and  2 for the length

    buff[size] = ']';

    char lengthStr[2];
    read(fd, &lengthStr, sizeof(char) * 2);
    buff[size + 1] = lengthStr[0];
    buff[size + 2] = lengthStr[1];
    size += 3;

    uint16_t length;
    memcpy(&length, lengthStr, 2);

    char c;
    for(int i = 0; i < length; i++, size++){
        read(fd, &c, sizeof(char));
        buff = (char *) realloc(buff, size + 1);
        buff[size] = c;
    }
    Packet *p = COM_deserializePacket(buff);
    free(buff);

    if(ipc) close(fd);

    if(!isPacketValid(p)){
        COM_freePacket(p);
        return NULL;
    }

    return p;
}


void COM_freePacket(Packet *packet){
    if(packet != NULL){
        if(packet->header != NULL) free(packet->header);
        if(packet->data != NULL) free(packet->data);
        free(packet);
    }
}


char * COM_serializePacket(Packet *p, size_t *packetSize){
    if(p == NULL) return NULL;

    *packetSize = 1 + strlen(p->header) + 2 + p->length;
    char *data = (char *) malloc(sizeof(char) * (*packetSize));
    memset(data, 0, sizeof(char) * *packetSize);

    data[0] = (char) p->type;
    strcat(data, p->header);

    char buff[2];
    memcpy(buff, &(p->length), sizeof(char) * 2);
    size_t currSize = strlen(data);
    data[currSize] = buff[0];
    data[currSize + 1] = buff[1];

    if(p->length > 0 && p->data != NULL){
        //Can't use strcat anymore, as one of the characters that represent the number could be '\0'
        for(size_t i = currSize+2, j = 0; i < *packetSize; i++, j++)
            data[i] = p->data[j];
    }

    return data;
}

Packet * COM_deserializePacket(const char *data){
    if(data == NULL) return NULL;

    Packet *p = (Packet *) malloc(sizeof(Packet));
    memset(p, 0, sizeof(Packet));
    char* buff = (char *) malloc(sizeof(char));

    p->type = data[0];
    size_t index = 1;
    size_t buffIndex = 0;
    int flag = 0;

    while(flag == 0){
        buff[buffIndex] = data[index];
        index++;
        buffIndex++;
        buff = (char *) realloc(buff, sizeof(char) * (buffIndex+1));
        if(data[index - 1] == ']' || data[index] == '\0') flag = 1;
    }
    buff[buffIndex] = '\0';
    p->header = buff;

    char lengthStr[2];
    lengthStr[0] = data[index];
    index++;
    lengthStr[1] = data[index];
    index++;
    memcpy(&(p->length), lengthStr, sizeof(char) * 2);

    if(p->length > 0){
        buffIndex = 0;
        buff = (char *) malloc(sizeof(char));

        while(buffIndex < p->length){
            buff[buffIndex] = data[index];
            index++;
            buffIndex++;
            buff = (char *) realloc(buff, sizeof(char) * (buffIndex+1));
        }
        buff[buffIndex] = '\0';
        p->data = buff;
    }
    else p->data = NULL;

    return p;
}


int COM_packetsRequired(int fileSize){
    if(fileSize <= 0) return 1;
    return fileSize/MAX_PACKET_SIZE + (fileSize % MAX_PACKET_SIZE == 0? 0: 1);
}

