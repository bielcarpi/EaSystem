#include "modules/ardaFunctions.h"

int main(int argc, char *argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGBUS, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGPIPE, signalHandler); //We already handle SIGPIPE checking all writes

    conf = NULL;
    connectionsHashmap = NULL;
    biggerFd = 0;
    connectionsList = NULL;

    if (argc != 2) {
        INOUT_print(ERR_ARGS);
        exit(EXIT_FAILURE);
    }

    //Read from file the total num of messages sent
    int fdFile = open(MESSAGES_COUNT_FILE_NAME, O_CREAT|O_RDONLY, 0640);
    if(read(fdFile, &numMessages, sizeof(int)) < 0) numMessages = 0;
    close(fdFile);

    //Parse the config file
    conf = FP_parseArdaConfig(argv[1]);

    //Start the server. Listen for new connections in fdListen
    if((fdListen = SCK_startServer(conf->ip, atoi(conf->port))) < 0){
        stop(1);
    };
    INOUT_print(SERVER_START);
    INOUT_print(READING_FILES);

    //Create the hashmap that will contain all connected users (structs of connection)
    connectionsHashmap = hashmap_new(sizeof(Connection), 0, 0, 0, CON_connectionHashByFDs, CON_connectionCompareFDs,
                                     CON_freeConnection, NULL);

    //Initialize fdset and add the fdListen to the fdset
    FD_ZERO(&fdset);
    FD_SET(fdListen, &fdset);
    biggerFd = fdListen;

    //Start the event loop in this main thread

    fd_set fdsetBuff;

    INOUT_print(WAITING_CONNECTIONS);

    //Event Loop
    for(;;){
        //Copy the fdset to a fdsetBuff (the one that will be modified)
        fdsetBuff = fdset;

        //Wait and save to fdsetBuff the fds that require attention
        select(biggerFd+1, &fdsetBuff, NULL, NULL, NULL);

        //Iterate over the fdsetBuff
        for(int i = 0; i < biggerFd+1; i++){
            if(FD_ISSET(i, &fdsetBuff)){
                if(i == fdListen) //If we're in the listen file descriptor
                    handleNewConnection();
                else //If the client is already connected, handle its petition
                    handleRequest(i);
            }
        }
    }
}
