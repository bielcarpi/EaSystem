#include "ardaFunctions.h"

void updateConnectionsList(){
    int intBuff;
    Connection **connections = NULL;
    CON_getConnectionsList(connectionsHashmap, &intBuff, &connections);
    free(connectionsList); //Free the old one
    connectionsList = CON_serializeConnections(intBuff, connections);
    free(connections);
}


void handleNewConnection(){
    //Accept the incoming connection
    int fdAccept = accept(fdListen, (struct sockaddr*) NULL, NULL);
    if(fdAccept < 0){
        INOUT_print(ERR_ACCEPT);
        return;
    }

    //If the server has reached MAX_CLIENTS, close the one accepted and wait for disconnections
    if(hashmap_count(connectionsHashmap) > MAX_CLIENTS){
        close(fdAccept);
        INOUT_print(SERVER_FULL);
        return;
    }

    Packet *p = COM_readPacket(0, fdAccept);
    //If the first packet received is invalid, or is not a NEW_SON packet, close the connection with KO response (firewall)
    //Interesting TODO: Have a list of IPs which tried to connect to the server unsuccessfully, and don't accept more connections from them
    if(!COM_isConnectionPacket(p)){
        COM_writePacket(CONKO, 0, NULL, 0, 0, fdAccept);
        close(fdAccept);
        COM_freePacket(p);
        return;
    }

    //If the first packet is OK, let's proceed. Parse the data in the Packet
    Connection *c = CON_deserializeConnection(p->data);
    c->fd = fdAccept;

    //Print new login
    char *buff;
    asprintf(&buff, NEW_LOGIN, c->userName, c->ip, c->port, c->pid, c->fd);
    INOUT_print(buff);
    free(buff);

    Connection *nc;
    //If there is another user with same name, disconnect
    if((nc = (Connection *) CON_getConnectionByName(connectionsHashmap, c->userName)) != NULL){
        //If the IP and port are the same, disconnect the old one and connect this one
        // else, don't allow the connection
        if(c->port == nc->port && strcmp(c->ip, nc->ip) == 0){
            INOUT_print(DISCONNECTING_OLD_SESSION);
            disconnect(nc->fd);
        }
        else{
            INOUT_print(ERR_USER_NAME);
            COM_writePacket(CONKO, 0, NULL, 0, 0, c->fd);
            COM_freePacket(p);
            CON_freeConnection(c);
            free(c);
            close(fdAccept);
            return;
        }
    }

    //Create the new client
    if(biggerFd < fdAccept) biggerFd = fdAccept;
    FD_SET(fdAccept, &fdset); //Put the file descriptor to the file descriptor set
    hashmap_set(connectionsHashmap, c); //Put the new Connection to the hashmap of connections

    //Update Connections List
    updateConnectionsList();

    //Send the Users List updated to the new Client
    handleListRequest(c->fd,1);

    free(c);
    COM_freePacket(p);
}


void handleRequest(int fd){
    Packet *p = COM_readPacket(0, fd);

    //Process the packet
    if(p == NULL){
        disconnect(fd); //Non-Blocking
    }
    else if(strcmp(p->header, LIST_REQUEST) == 0){
        handleListRequest(fd,0); //Non-Blocking
    }
    else if(strcmp(p->header, NEW_MSG) == 0){
        numMessages++; //Non-blocking
    }
    else if(strcmp(p->header, EXIT) == 0){
        if(p->data == NULL || strcmp(p->data, ((Connection*)hashmap_get(connectionsHashmap, &(Connection){.fd = fd}))->userName) != 0){
            INOUT_print(p->data == NULL? "null": p->data);
            INOUT_print(((Connection*)hashmap_get(connectionsHashmap, &(Connection){.fd = fd}))->userName);
            COM_writePacket(CONKO, 0, NULL, 1, 0, fd);
        }
        else
            disconnect(fd); //Non-Blocking
    }
    else{
        handleUnknownRequest(fd); //Non-Blocking
    }

    //Once handled, free the packet
    COM_freePacket(p);
}



void handleListRequest(int fd, int newConnection){
    char *buff;

    if(newConnection) asprintf(&buff, USER_LIST_REQUEST);
    else asprintf(&buff, USER_LIST_REQUEST_UPDATE, fd, ((Connection*)hashmap_get(connectionsHashmap, &(Connection){.fd = fd}))->userName);
    INOUT_print(buff);
    free(buff);

    if(COM_writePacket(CONOK, strlen(connectionsList), connectionsList, 0, 0, fd) < 0){
        INOUT_print(ERR_WRITE);
        disconnect(fd); //As we can't write to it, disconnect that Connection
    }
    else{
        if(!newConnection){
            asprintf(&buff, RESPONSE_SENT_UPDATE, ((Connection*)hashmap_get(connectionsHashmap, &(Connection){.fd = fd}))->userName);
            INOUT_print(buff);
            free(buff);
        }
        else INOUT_print(RESPONSE_SENT);
    }
}



void handleUnknownRequest(int fd){
    char* buff;

    asprintf(&buff, UNKNOWN_REQUEST, fd);
    INOUT_print(buff);
    free(buff);

    if(COM_writePacket(UNKNOWN, 0, NULL, 0, 0, fd) < 0){
        INOUT_print(ERR_WRITE);
        disconnect(fd); //As we can't write to it, disconnect that Connection
    }
    else INOUT_print(RESPONSE_SENT);
}


void disconnect(int fd){
    Connection *c = hashmap_get(connectionsHashmap, &(Connection){.fd = fd});

    //Clear the fd from the fdset and remove the Connection from the hashmap
    FD_CLR(fd, &fdset);
    hashmap_delete(connectionsHashmap, c);

    //Write the exit packet
    COM_writePacket(CONOK, 0, NULL, 1, 0, fd);
    close(fd);

    //Update Connections List
    updateConnectionsList();

    char* buff;
    asprintf(&buff, DISCONNECTED, c->userName, c->ip, c->port, c->pid, c->fd);
    INOUT_print(buff);
    free(buff);
    CON_freeConnection(c);
}


void signalHandler(int sigNum){
    if(sigNum == SIGPIPE){
        INOUT_print(ERR_WRITE);
        signal(SIGPIPE, signalHandler);
        return;
    }

    char *buff;
    asprintf(&buff, EXIT_SIGNAL, strsignal(sigNum));
    INOUT_print(buff);
    free(buff);

    stop(1);
}


void stop(int exitCode){
    char *buff;

    if(fdListen >= 0) close(fdListen);
    FD_CLR(fdListen, &fdset);

    //Disconnect all fds in fdset
    for(int i = 0; i < biggerFd+1; i++)
        if(FD_ISSET(i, &fdset)) close(i);

    if(conf != NULL) FP_freeArdaConfig(conf);
    hashmap_free(connectionsHashmap);
    if(connectionsList != NULL) free(connectionsList);

    //Write to file the total num of messages sent
    int fdFile = open(MESSAGES_COUNT_FILE_NAME, O_WRONLY);
    write(fdFile, &numMessages, sizeof(int));
    close(fdFile);

    asprintf(&buff, STOP_SUCCESS, numMessages);
    INOUT_print(buff);
    free(buff);

    INOUT_print(""); //Reset color
    INOUT_end();
    exit(exitCode);
}
