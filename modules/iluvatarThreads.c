#include "iluvatarThreads.h"

static void initThread(){
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

static void stopThread(ThreadArgs *threadArgs){
    ITH_freeThreadArgs(threadArgs);
    free(threadArgs);
    INOUT_print(PS2); //Print again PS2
}


int ITH_threadArgsCompareIDs(const void *a, const void *b, void *udata){
    UNUSED(udata); //Needed for the hashmap
    const ThreadArgs *ca = a;
    const ThreadArgs *cb = b;
    return ((long)ca->threadId) != ((long)cb->threadId); //0 true, else false
}


uint64_t ITH_threadArgsHashByIDs(const void *item, uint64_t seed0, uint64_t seed1){
    ThreadArgs *ta = (ThreadArgs *) item;
    char *buff;
    asprintf(&buff, "%ld", (long)(ta->threadId));
    uint64_t sip = hashmap_sip(buff, strlen(buff), seed0, seed1);
    free(buff);
    return sip;
}


void ITH_getThreadArgsList(struct hashmap *map, int *numThreadArgs, ThreadArgs ***threadArgs){
    *threadArgs = (ThreadArgs **) malloc(sizeof(ThreadArgs *));
    size_t iter = 0, i = 0;
    void *item;
    while (hashmap_iter(map, &iter, &item)) {
        ThreadArgs *args = item;
        if(i != 0) *threadArgs = (ThreadArgs **) realloc(*threadArgs, sizeof(ThreadArgs *) * (i+1));
        (*threadArgs)[i] = args;
        i++;
    }

    *numThreadArgs = (int) i;
}


static void receiveFile(ThreadArgs *threadArgs, int fdSocket, int fdIpc[], int ipc){
    Connection *sender = threadArgs->c;
    IluvatarSonConfig *config = threadArgs->config;

    char *buff;
    int stop = 0;
    File *f = MS_deserializeFile(threadArgs->p->data);

    if(ipc)
        asprintf(&buff, RECEIVED_FILE_NEIGHBOUR, f->originUser, f->fileName, config->folder, f->fileName);
    else
        asprintf(&buff, RECEIVED_FILE, f->originUser, sender == NULL? "?": sender->ip, sender == NULL? "?": sender->domainName, f->fileName, config->folder, f->fileName);
    INOUT_print(buff);
    free(buff);
    INOUT_print(PS2);

    asprintf(&buff, "%s/%s", &(config->folder[1]), f->fileName);
    int fdFile = open(buff, O_CREAT|O_TRUNC|O_RDWR, 0644);
    if(fdFile >= 0 && flock(fdFile, LOCK_EX | LOCK_NB) < 0){ //Exclusive file lock (only WE will use the file)
        INOUT_print(ERR_FILE_OPEN);
        close(fdFile);
        fdFile = -1;
    }

    int packetsRequired = COM_packetsRequired(atoi(f->fileSize));
    Packet *pb;
    for(int i = 0; i < packetsRequired; i++){
        pb = COM_readPacket(ipc, ipc? fdIpc[1]: fdSocket);

        pthread_rwlock_rdlock(threadArgs->mtxRun);
        if(pb == NULL || *(threadArgs->run) == 0){ //If we must stop running
            pthread_rwlock_unlock(threadArgs->mtxRun);

            char *str;
            asprintf(&str, STOP_RECEIVING_FILE, f->fileName, f->originUser);
            INOUT_print(str);
            free(str);

            COM_freePacket(pb);
            stop = 1;
            break;
        }
        pthread_rwlock_unlock(threadArgs->mtxRun);

        if(fdFile >= 0) write(fdFile, pb->data, pb->length);
        COM_freePacket(pb);
    }

    if(fdFile < 0){
        INOUT_print(ERR_WRITING_FILE);
        COM_writePacket(CHECK_KO, 0, NULL, 0, ipc, ipc? fdIpc[0]: fdSocket);
        MS_freeFile(f);
        free(buff);
        return;
    }

    if(!stop){
        char *hash = SHH_getHashMD5(buff);
        free(buff);
        if(hash != NULL && strcmp(hash, f->hashDM5) == 0){
            asprintf(&buff, FILE_CORRECTLY_RECEIVED, f->fileName, f->originUser);
            COM_writePacket(CHECK_OK, 0, NULL, 0, ipc, ipc? fdIpc[0]: fdSocket);
        }
        else{
            asprintf(&buff, FILE_INCORRECTLY_RECEIVED, f->fileName, f->originUser);
            COM_writePacket(CHECK_KO, 0, NULL, 0, ipc, ipc? fdIpc[0]: fdSocket);
        }
        INOUT_print(buff);
        free(hash);
    }

    flock(fdFile, LOCK_UN); //Unlock fdFile
    close(fdFile);
    MS_freeFile(f);
    free(buff);
}


void* ITH_sendMessage(void *args){
    ThreadArgs *threadArgs = (ThreadArgs *) args;
    initThread();
    Message *m = threadArgs->m;
    Connection *c = threadArgs->c;
    IluvatarSonConfig *config = threadArgs->config;
    int ipc = threadArgs->ipc;
    int fdArda = threadArgs->fd;
    char *message = MS_serializeMessage(m);

    if(ipc){ //Send message through IPC
        char *id = IPC_sendAwakeMessage((int) c->pid);
        if(id == NULL){
            INOUT_print(ERR_CONN_ILUVATAR);
            free(message);
            stopThread(threadArgs);
            return NULL;
        }
        int *fds = IPC_startQueues(id); //0 for reading, 1 for writing
        COM_writePacket(SEND_MSG, strlen(message), message, 0, 1, fds[1]);

        Packet *p = COM_readPacket(1, fds[0]);
        if(p != NULL && strcmp(p->header, MSGOK) == 0) INOUT_print(MSG_CORRECTLY_SENT);
        else INOUT_print(ERR_SENDING_MSG);

        IPC_stopQueues(id, fds);
        COM_freePacket(p);
    }
    else{ //Send message through Sockets
        //Create the socket to the corresponding Iluvatar
        int fdIluvatar;
        if((fdIluvatar = SCK_connectToServer(c->ip, c->port)) < 0){
            free(message);
            stopThread(threadArgs);
            return NULL;
        }

        COM_writePacket(SEND_MSG,strlen(message), message, 0, 0, fdIluvatar);

        Packet *p = COM_readPacket(0, fdIluvatar);
        if(p != NULL && strcmp(p->header, MSGOK) == 0) INOUT_print(MSG_CORRECTLY_SENT);
        else INOUT_print(ERR_SENDING_MSG);
        COM_freePacket(p);
        close(fdIluvatar);
    }

    free(message);

    //Finally, write packet to Arda to keep track of total num of messages sent through the network
    pthread_mutex_lock(threadArgs->mtxFdArda);
    COM_writePacket(NEW_MSG, strlen(config->name), config->name, 0, 0, fdArda);
    pthread_mutex_unlock(threadArgs->mtxFdArda);

    stopThread(threadArgs);
    return NULL;
}


void* ITH_sendFile(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *) args;
    initThread();

    File *f = threadArgs->f;
    Connection *c = threadArgs->c;
    IluvatarSonConfig *config = threadArgs->config;
    int ipc = threadArgs->ipc;

    char *buff;
    char *writeBuff, *queueId;
    int *fds, fdIluvatar;
    int currentPacketSize;
    int stop = 0;

    //Open file to read
    asprintf(&buff, "%s/%s", &(config->folder[1]), f->fileName);
    int fdFile = open(buff, O_RDWR);
    if(fdFile < 0){
        INOUT_print(ERR_FILE_EXIST);
        free(buff);
        stopThread(threadArgs);
        return NULL;
    }
    if(flock(fdFile, LOCK_EX | LOCK_NB) < 0){ //Exclusive file lock (only we will write)
        INOUT_print(ERR_FILE_OPEN);
        close(fdFile);
        free(buff);
        stopThread(threadArgs);
        return NULL;
    }

    //Get file size
    asprintf(&(f->fileSize), "%ld", lseek(fdFile, 0, SEEK_END));
    lseek(fdFile, 0, SEEK_SET);

    //Run command MD5SUM to get the hash
    f->hashDM5 = SHH_getHashMD5(buff);
    free(buff);

    //Check that file has a valid extension
    buff = strrchr(f->fileName, '.'); //Get last .(and something) of string
    if(!buff || strcmp(buff, f->fileName) == 0){
        INOUT_print(ERR_FILE_EXTENSION);
        free(buff);
        close(fdFile);
        stopThread(threadArgs);
        return NULL;
    }

    //Serialize File (get only the name of the file, not its path)
    buff = f->fileName;
    f->fileName = SHH_getBasename(buff);
    char *serializedFile = MS_serializeFile(f);
    free(f->fileName);
    f->fileName = buff;

    int packetsRequired = COM_packetsRequired(atoi(f->fileSize));

    if(ipc){
        queueId = IPC_sendAwakeMessage((int) c->pid);
        if(queueId == NULL){
            INOUT_print(ERR_CONN_ILUVATAR);
            stopThread(threadArgs);
            return NULL;
        }
        fds = IPC_startQueues(queueId); //0 for reading, 1 for writing
    }
    else{
        if((fdIluvatar = SCK_connectToServer(c->ip, c->port)) < 0){
            stopThread(threadArgs);
            return NULL;
        }
    }

    //Write the SEND_FILE packet
    COM_writePacket(SEND_FILE, strlen(serializedFile), serializedFile, 0, ipc, ipc? fds[1]: fdIluvatar);
    asprintf(&buff, SENDING_FILE, f->fileName,c->userName);
    INOUT_print(buff);
    free(buff);
    free(serializedFile);
    INOUT_print(PS2);

    int sent;
    //Write all the other packets containing the FILE_DATA
    for(int i = 0; i < packetsRequired; i++){
        writeBuff = (char *) malloc(sizeof(char) * MAX_PACKET_SIZE);
        memset(writeBuff, 0, MAX_PACKET_SIZE);
        currentPacketSize = read(fdFile, writeBuff, MAX_PACKET_SIZE);
        sent = COM_writePacket(FILE_DATA, currentPacketSize, writeBuff, 0, ipc, ipc? fds[1]: fdIluvatar);
        free(writeBuff);

        pthread_rwlock_rdlock(threadArgs->mtxRun);
        if(sent == -1 || *(threadArgs->run) == 0){ //If we must stop running
            pthread_rwlock_unlock(threadArgs->mtxRun);

            char *str;
            asprintf(&str, STOP_SENDING_FILE, f->fileName, c->userName);
            INOUT_print(str);
            free(str);

            stop = 1;
            break;
        }
        pthread_rwlock_unlock(threadArgs->mtxRun);
    }

    if(!stop){
        Packet *p = COM_readPacket(ipc, ipc? fds[0]: fdIluvatar);

        if(p == NULL)
            asprintf(&buff, END_RECEIVING_FILE, c->userName, f->fileName);
        else if(strcmp(p->header, CHECK_OK) == 0)
            asprintf(&buff, FILE_CORRECTLY_SENT, f->fileName, c->userName);
        else
            asprintf(&buff, FILE_INCORRECTLY_SENT, f->fileName, c->userName);
        INOUT_print(buff);
        free(buff);

        COM_freePacket(p); //Free last packet received
    }

    if(ipc) //If we went through IPC, close IPC resources
        IPC_stopQueues(queueId, fds);
    else //If we sent through sockets, close Socket resources
        close(fdIluvatar);

    flock(fdFile, LOCK_UN); //Unlock fdFile
    close(fdFile);

    stopThread(threadArgs);
    return NULL;
}


void* ITH_checkSocketMessage(void *args){
    ThreadArgs *threadArgs = (ThreadArgs *) args;
    initThread();
    Connection *sender = threadArgs->c;
    Packet *p = threadArgs->p;
    int fdAccept = threadArgs->fd;

    char *buff;

    if(p != NULL && strcmp(p->header, SEND_MSG) == 0){
        Message *msg = MS_deserializeMessage(p->data);
        asprintf(&buff, RECEIVED_MSG, msg->originUser, sender == NULL? "?": sender->ip,
                 sender == NULL? "?": sender->domainName, msg->message);
        INOUT_print(buff);
        free(buff);

        COM_writePacket(MSGOK, 0, NULL, 0, 0, fdAccept);
        MS_freeMessage(msg);
    }
    else if(p != NULL && strcmp(p->header, SEND_FILE) == 0){
        receiveFile(threadArgs, fdAccept, NULL, 0);
    }
    else{
        asprintf(&buff, UNKNOWN_PACKET_ILUVATAR, p == NULL? "NULL": p->header);
        INOUT_print(buff);
        free(buff);
    }

    close(fdAccept);

    stopThread(threadArgs);
    return NULL;
}


void* ITH_checkIpcMessage(void *args){
    ThreadArgs *threadArgs = (ThreadArgs *) args;
    initThread();
    char *buff;

    int *fds = IPC_startQueues(threadArgs->idIpc); //0 for writing, 1 for reading
    Packet *p = COM_readPacket(1, fds[1]);
    threadArgs->p = p;

    if(p != NULL && strcmp(p->header, SEND_MSG) == 0){
        Message *msg = MS_deserializeMessage(p->data);
        asprintf(&buff, RECEIVED_MSG_NEIGHBOUR, msg->originUser, msg->message);
        INOUT_print(buff);
        free(buff);

        COM_writePacket(MSGOK, 0, NULL, 0, 1, fds[0]);
        MS_freeMessage(msg);
    }
    else if(p != NULL && strcmp(p->header, SEND_FILE) == 0){
        receiveFile(threadArgs, -1, fds, 1);
    }
    else{
        asprintf(&buff, UNKNOWN_PACKET_IPC, p == NULL? "NULL": p->header);
        INOUT_print(buff);
        free(buff);
    }

    IPC_stopQueues(threadArgs->idIpc, fds);

    stopThread(threadArgs);
    return NULL;
}


void ITH_freeThreadArgs(ThreadArgs *args){
    if(args != NULL){
        if(args->f != NULL) MS_freeFile(args->f);
        if(args->m != NULL) MS_freeMessage(args->m);
        if(args->p != NULL) COM_freePacket(args->p);
    }
}
