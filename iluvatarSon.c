#include "modules/iluvatarFunctions.h"

int main(int argc, char *argv[]) {
    fdArda = -1;
    fdSocket = -1;
    fdIpc = -1;
    run = 1;
    threadsHashmap = NULL;
    connectionsHashmap = NULL;
    char *buff;
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGBUS, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGPIPE, signalHandler);

    if(argc != 2){
        INOUT_print(ERR_ARGUMENTS);
        end(0, 1);
    }

    //Parse the configuration for this Iluvatar
    config = FP_parseIluvatarSonConfig(argv[1]);
    if(config == NULL){
        INOUT_print(ERR_READING);
        end(0, 1);
    }

    //If file has been read correctly, continue
    asprintf(&buff, WELCOME_MSG, config->name);
    INOUT_print(buff);
    free(buff);


    //Start the IPC file descriptor
    if((fdIpc = IPC_start()) < 0){
        INOUT_print(IPC_ERR_CRT);
        end(0, 1);
    }

    //Start the Server Socket file descriptor
    if((fdSocket = SCK_startServer(config->ip, atoi(config->port))) < 0){
        end(0, 1);
    }

    //Now that we've initialized fdSocket and fdIpc correctly, try to connect to Arda
    INOUT_print(CONNECTING_TO_ARDA);
    if((fdArda = SCK_connectToServer(config->ipArda, atoi(config->portArda))) < 0){
        end(0, 1);
    }
    INOUT_print(ARDA_CONNECTION_SUCCESS);

    //Create the hashmap that will contain all connected users (structs of connection)
    connectionsHashmap = hashmap_new(sizeof(Connection), 0, 0, 0, CON_connectionHashByNames, CON_connectionCompareNames,
                                     CON_freeConnection, NULL);

    //Create the hashmap that will contain all threads (structs of ThreadArgs)
    threadsHashmap = hashmap_new(sizeof(ThreadArgs), 0, 0, 0, ITH_threadArgsHashByIDs, ITH_threadArgsCompareIDs,
                                     NULL, NULL);

    //Start mutex semaphores
    pthread_rwlock_init(&mtxRun, NULL);
    pthread_mutex_init(&mtxFdArda, NULL);

    //Now that we're connected to arda, send our info
    sendNewSon();

    //Start the Iluvatar
    char *formattedBuff;
    fd_set fdset, fdsetBuff;

    //Save in a fdset STDIN, fdArda, fdSocket and fdIpc
    FD_ZERO(&fdset);
    FD_SET(STDIN_FILENO, &fdset);
    FD_SET(fdIpc, &fdset);
    FD_SET(fdSocket, &fdset);
    FD_SET(fdArda, &fdset);

    for(;;){
        INOUT_print(PS2);
        fdsetBuff = fdset;
        select(fdArda+1, &fdsetBuff, NULL, NULL, NULL);

        if(FD_ISSET(fdArda, &fdsetBuff)){ //If arda has sent us something...
            checkArdaMessage();
            continue;
        }

        if(FD_ISSET(fdSocket, &fdsetBuff)){ //If somebody has sent us something through Sockets...
            checkSocketMessage();
            continue;
        }

        if(FD_ISSET(fdIpc, &fdsetBuff)){ //If somebody has sent us something through IPC...
            checkIpcMessage();
            continue;
        }

        //Else, we'll have something in STDIN_FILENO
        INOUT_readStdin(&formattedBuff,&buff);


        //Check what's been introduced
        if(strcmp(formattedBuff, UPDATE_USERS_TEXT) == 0)
            updateUsers();
        else if(strcmp(formattedBuff, LIST_USERS_TEXT) == 0)
            listUsers();
        else if(strstr(formattedBuff, SEND_MSG_TEXT) != NULL)
            prepareSendMsg(buff);
        else if(strstr(formattedBuff, SEND_FILE_TEXT) != NULL)
            prepareSendFile(buff);
        else if(strcmp(formattedBuff, HELP_TEXT) == 0)
            INOUT_print(HELP_MSG);
        else if(strcmp(formattedBuff, EXIT_PROGRAM_TEXT) == 0){
            free(buff);
            free(formattedBuff);
            end(1, 0);
            continue; //If we haven't ended, continue
        }
        else if(strstr(formattedBuff, "list") != NULL)
            INOUT_print(POSSIBLE_ERR_LIST);
        else if(strstr(formattedBuff, "update") != NULL)
            INOUT_print(POSSIBLE_ERR_UPDATE);
        else if(strstr(formattedBuff, "send") != NULL)
            INOUT_print(POSSIBLE_ERR_SEND);
        else
            SHH_runCommand(buff); //Run what we've introduced with the shell

        free(buff);
        free(formattedBuff);
    }

}
