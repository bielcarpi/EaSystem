#include "socketHelper.h"

int SCK_startServer(char *ip, int port){
    int fdListen;
    struct sockaddr_in server;

    //Create the Listen Socket
    if( (fdListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        INOUT_print(ERR_CRT_SOCKET);
        return -1;
    }

    //Prepare the sockaddr_in server structure
    memset(&server, 0, sizeof(server));
    server.sin_port = htons(port);
    server.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &server.sin_addr) < 0) {
        INOUT_print(ERR_CONF_IP);
        close(fdListen);
        return -1;
    }

    //Bind
    if(bind(fdListen, (struct sockaddr*) &server, sizeof(server)) < 0){
        INOUT_print(ERR_BIND);
        close(fdListen);
        return -1;
    }

    //Listen the Socket
    if(listen(fdListen, LISTENING_QUEUE) < 0){
        INOUT_print(ERR_LIST);
        close(fdListen);
        return -1;
    }

    return fdListen;
}


int SCK_connectToServer(char *ip, int port){
    struct sockaddr_in server;
    int fdConnection;

    if((fdConnection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        INOUT_print(ERR_CRT_SOCKET);
        return -1;
    }
    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if(inet_pton(AF_INET, ip, &server.sin_addr) < 0){
        INOUT_print(ERR_CONF_IP);
        close(fdConnection);
        return -1;
    }
    if(connect(fdConnection, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) < 0){
        INOUT_print(ERR_CONN);
        close(fdConnection);
        return -1;
    }

    return fdConnection;
}