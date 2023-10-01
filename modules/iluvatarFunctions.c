#include "iluvatarFunctions.h"

void sendNewSon(){
    Connection *c = &(Connection){.userName = config->name, .ip = config->ip, .port = atoi(config->port), .pid = getpid()};
    char *data = CON_serializeConnection(c);
    COM_writePacket(NEW_SON, strlen(data), data, 0, 0, fdArda);
    free(data);

    Packet *p = COM_readPacket(0, fdArda);
    if(p == NULL){
        INOUT_print(ERR_CONN_ARDA);
        end(0, EXIT_FAILURE);
    }
    else if(strcmp(p->header, CONOK) != 0){
        //Arda don't want us to connect. Free everything and exit
        INOUT_print(ERR_CONN_ARDA);
        COM_freePacket(p);
        close(fdArda);
        end(0, EXIT_FAILURE);
        return;
    }

    Connection **connections = NULL;
    int numConnections = 0;
    CON_deserializeConnections(p->data, &numConnections, &connections);

    hashmap_clear(connectionsHashmap, false);
    for(int i = 0; i < numConnections; i++){
        connections[i]->domainName = SHH_getDomain(connections[i]->ip);
        hashmap_set(connectionsHashmap, connections[i]);
        free(connections[i]);
    }

    free(connections);
    COM_freePacket(p);
    INOUT_print(UPDATE_USERS_MSG);
}


void updateUsers(){
    pthread_mutex_lock(&mtxFdArda);
    Connection *c = &(Connection){.userName = config->name, .ip = config->ip, .port = atoi(config->port), .pid = getpid()};
    COM_writePacket(LIST_REQUEST, strlen(c->userName), c->userName, 0, 0, fdArda);

    Packet *p = COM_readPacket(0, fdArda);
    if(p == NULL){
        INOUT_print(ERR_CONN_ARDA);
        end(0, EXIT_FAILURE);
    }

    Connection **connections = NULL;
    int numConnections = 0;
    CON_deserializeConnections(p->data, &numConnections, &connections);

    hashmap_clear(connectionsHashmap, false);
    for(int i = 0; i < numConnections; i++){
        connections[i]->domainName = SHH_getDomain(connections[i]->ip);
        hashmap_set(connectionsHashmap, connections[i]);
        free(connections[i]);
    }

    free(connections);
    COM_freePacket(p);
    INOUT_print(UPDATE_USERS_MSG);
    pthread_mutex_unlock(&mtxFdArda);
}


void listUsers(){
    char *buff;
    Connection **connections = NULL;
    int numConnections;

    CON_getConnectionsList(connectionsHashmap, &numConnections, &connections);
    asprintf(&buff, LIST_USERS_MSG, numConnections);
    INOUT_print(buff);
    free(buff);

    for(int i = 0; i < numConnections; i++){
        asprintf(&buff, LIST_USERS_LINE_MSG, i+1, connections[i]->userName, connections[i]->ip, connections[i]->domainName? connections[i]->domainName: "?", connections[i]->port, connections[i]->pid);
        INOUT_print(buff);
        free(buff);
    }

    free(connections);
    INOUT_print("\n");
}


void prepareSendMsg(char *buff){
    Message *m = (Message *) FP_parseMsgFile(buff);
    if(m == NULL){
        INOUT_print(ERR_MSG_INVALID);
        return;
    }

    if(strlen(m->message) > 450){
        INOUT_print(ERR_MSG_LONG);
        MS_freeMessage(m);
        return;
    }

    Connection* c = hashmap_get(connectionsHashmap, &(Connection){.userName = m->originUser});
    if(c == NULL){
        INOUT_print(ERR_USER_NAME);
        MS_freeMessage(m);
        return;
    }
    else if(strcmp(c->userName, config->name) == 0){ //If we're sending a message to ourselves
        INOUT_print(ERR_MSG_SELF);
        MS_freeMessage(m);
        return;
    }

    //Now that we've identified the user we want to send the message to (c), change origin user
    free(m->originUser);
    m->originUser = strdup(config->name);

    ThreadArgs *threadArgs = (ThreadArgs *) malloc(sizeof(ThreadArgs));
    memset(threadArgs, 0, sizeof(ThreadArgs));
    threadArgs->m = m;
    threadArgs->c = c;
    threadArgs->config = config;
    threadArgs->fd = fdArda;
    threadArgs->mtxFdArda = &mtxFdArda;
    threadArgs->ipc = IPC_shouldUseIPC(config->ip, c); //If we should use IPC as we're in the same machine, use it

    //Start the thread that will carry out the message sending
    pthread_create(&(threadArgs->threadId), NULL, ITH_sendMessage, (void*)threadArgs);
    hashmap_set(threadsHashmap, threadArgs);
}


void prepareSendFile(char *buff){
    File *f = (File *) FP_parseMsgFile(buff);
    if(f == NULL){
        INOUT_print(ERR_FILE_INVALID);
        return;
    }

    Connection* c = hashmap_get(connectionsHashmap, &(Connection){.userName = f->originUser});
    if(c == NULL){
        INOUT_print(ERR_USER_NAME);
        MS_freeFile(f);
        return;
    }
    else if(strcmp(c->userName, config->name) == 0) { //If we're sending a file to ourselves
        INOUT_print(ERR_FILE_SELF);
        MS_freeFile(f);
        return;
    }

    //Now that we've identified the user we want to send the file to (c), change origin user
    free(f->originUser);
    f->originUser = strdup(config->name);

    ThreadArgs *threadArgs = (ThreadArgs *) malloc(sizeof(ThreadArgs));
    memset(threadArgs, 0, sizeof(ThreadArgs));
    threadArgs->f = f;
    threadArgs->c = c;
    threadArgs->config = config;
    threadArgs->mtxRun = &mtxRun;
    threadArgs->run = &run;
    threadArgs->ipc = IPC_shouldUseIPC(config->ip, c); //If we should use IPC as we're in the same machine, use it

    //Start the thread that will carry out the file sending
    pthread_create(&(threadArgs->threadId), NULL, ITH_sendFile, (void*)threadArgs);
    hashmap_set(threadsHashmap, threadArgs);
}


void checkSocketMessage() {
    int fdAccept = accept(fdSocket, (struct sockaddr *) NULL, NULL);
    if (fdAccept < 0) return;
    Packet *p = COM_readPacket(0, fdAccept);
    Connection *sender = NULL;

    if(p!= NULL && strcmp(p->header, SEND_MSG) == 0){
        Message *m = MS_deserializeMessage(p->data);
        sender = hashmap_get(connectionsHashmap, &(Connection){.userName = m->originUser});
        MS_freeMessage(m);
    }
    else if(p != NULL && strcmp(p->header, SEND_FILE) == 0){
        File *f = MS_deserializeFile(p->data);
        sender = hashmap_get(connectionsHashmap, &(Connection){.userName = f->originUser});
        MS_freeFile(f);
    }

    ThreadArgs *threadArgs = (ThreadArgs *) malloc(sizeof(ThreadArgs));
    memset(threadArgs, 0, sizeof(ThreadArgs));
    threadArgs->p = p;
    threadArgs->c = sender;
    threadArgs->fd = fdAccept;
    threadArgs->config = config;
    threadArgs->run = &run;
    threadArgs->mtxRun = &mtxRun;

    //Start the thread that will carry out the checking
    pthread_create(&(threadArgs->threadId), NULL, ITH_checkSocketMessage, (void*)threadArgs);
    hashmap_set(threadsHashmap, threadArgs);
}


void checkIpcMessage(){
    char *idIpc = IPC_readAwakeMessage(fdIpc);

    ThreadArgs *threadArgs = (ThreadArgs *) malloc(sizeof(ThreadArgs));
    memset(threadArgs, 0, sizeof(ThreadArgs));
    threadArgs->idIpc = idIpc;
    threadArgs->config = config;
    threadArgs->run = &run;
    threadArgs->mtxRun = &mtxRun;

    //Start the thread that will carry out the checking
    pthread_create(&(threadArgs->threadId), NULL, ITH_checkIpcMessage, (void*)threadArgs);
    hashmap_set(threadsHashmap, threadArgs);
}


void checkArdaMessage(){
    pthread_mutex_lock(&mtxFdArda);
    char *buff;
    Packet *p = COM_readPacket(0, fdArda);

    if(p == NULL || strcmp(p->header, CONOK) == 0){
        INOUT_print(ERR_ARDA_CLOSED);
        close(fdArda);
        fdArda = -1;
        end(0, EXIT_SUCCESS);
    }
    else{
        asprintf(&buff, UNKNOWN_PACKET_ARDA, p->header);
        INOUT_print(buff);
        free(buff);
    }

    COM_freePacket(p);
    pthread_mutex_unlock(&mtxFdArda);
}


void signalHandler(int signal){
    char *buff;
    asprintf(&buff, EXIT_SIGNAL, strsignal(signal));
    INOUT_print(buff);
    free(buff);

    end(1, 1);
}


/**
 * Ends the program freeing all the memory, with the code introduced
 * @param msg Whether a goodbye message should be displayed (1) or not (0)
 * @param code The code with which the program will end
 */
void end(int msg, int code){
    if(msg) INOUT_print(EXIT_MSG);
    if(fdArda >= 0){
        pthread_mutex_lock(&mtxFdArda);
        if(COM_writePacket(EXIT, strlen(config->name), config->name, 1, 0, fdArda) > 0) {
            Packet *p = COM_readPacket(0, fdArda);
            if(p != NULL){
                int exitOk = strcmp(p->header, CONOK);
                COM_freePacket(p);
                if(exitOk != 0) {
                    INOUT_print(ERR_DISCONNECTION);
                    return; //Don't exit
                }
            }
        }
        close(fdArda);
        fdArda = -1;
        pthread_mutex_unlock(&mtxFdArda);
    }

    if(threadsHashmap != NULL){
        //End all threads
        pthread_rwlock_wrlock(&mtxRun);
        run = 0;
        pthread_rwlock_unlock(&mtxRun);

        ThreadArgs **threadArgsList;
        int numThreads;
        ITH_getThreadArgsList(threadsHashmap, &numThreads, &threadArgsList);

        INOUT_print(STOPPING_THREADS);

        for(int i = 0; i < numThreads; i++){
            pthread_join(threadArgsList[i]->threadId, NULL); //Wait for all threads to end, now that we've sent run=0
            pthread_detach(threadArgsList[i]->threadId); //Wait for all threads to end, now that we've sent run=0
            hashmap_delete(threadsHashmap, threadArgsList[i]);
        }

        free(threadArgsList);
    }

    if(fdSocket >= 0) close(fdSocket);
    if(fdIpc >= 0) IPC_stop(fdIpc);

    INOUT_print(""); //Reset color of stdout
    INOUT_end();
    pthread_rwlock_destroy(&mtxRun);
    pthread_mutex_destroy(&mtxFdArda);
    hashmap_free(connectionsHashmap);
    hashmap_free(threadsHashmap);
    FP_freeIluvatarSonConfig(config);
    exit(code);
}