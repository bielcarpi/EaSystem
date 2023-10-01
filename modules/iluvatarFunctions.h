#ifndef EASYSTEM_ILUVATARFUNCTIONS_H
#define EASYSTEM_ILUVATARFUNCTIONS_H

#include "iluvatarThreads.h"

#define ERR_ARGUMENTS RED"Error. Please, introduce a valid number of arguments."
#define ERR_READING RED"Error reading the config file. Please, introduce a valid file\n"
#define EXIT_SIGNAL RED"\nTerminating due to signal: %s\x1B[0m"
#define EXIT_MSG RED"\nDisconnecting from Arda. See you soon, son of Iluvatar\n\n"
#define WELCOME_MSG GRN"\nWelcome %s, son of Iluvatar\n"
#define UPDATE_USERS_TEXT "updateusers"
#define LIST_USERS_TEXT "listusers"
#define SEND_MSG_TEXT "sendmsg"
#define SEND_FILE_TEXT "sendfile"
#define HELP_TEXT "help"
#define EXIT_PROGRAM_TEXT "exit"
#define UPDATE_USERS_MSG "Users list updated\n\n"
#define LIST_USERS_MSG "There are %d children of Iluvatar Connected\n"
#define LIST_USERS_LINE_MSG "%d. " CYN COLOR_BOLD "%s" RESET " %s " GRN "(%s)" RESET " %d %d\n"
#define ERR_FILE_SELF RED"Error. You can't send a File to yourself.\n\n"
#define ERR_MSG_SELF RED"Error. You can't send a Message to yourself.\n\n"
#define IPC_ERR_CRT RED"Error creating the IPC queue. IPC will not work.\n\n"
#define UNKNOWN_PACKET_ARDA RED"\nUnknown Arda packet received, with header %s\n\n"
#define CONNECTING_TO_ARDA "Trying to connect to Arda...\n"
#define ARDA_CONNECTION_SUCCESS "Connected to" BLU COLOR_BOLD " Arda " RESET "successfully!\n"
#define ERR_USER_NAME RED"There is no user with this username.\n\n"
#define ERR_CONN_ARDA RED"\nError Connecting to Arda. Arda sent us a CONKO.\n\n"
#define ERR_ARDA_CLOSED RED"\nError. Arda has closed the connection.\n\n"
#define ERR_MSG_LONG RED"\nError. You can't send messages with more than 450 characters.\n\n"
#define ERR_MSG_INVALID RED"\nError. The message is not valid. Did you specify a user and write the message between \"\"?" RESET "\nExample: send msg Galadriel \"Hi Galdriel!\"\n\n"
#define ERR_DISCONNECTION RED"\nError. Arda won't let us disconnect.\n\n"
#define HELP_MSG CYN"HELP MENU\n" RESET "You can:\n -> 'list users' (send the list of connected users)\n -> 'update users' (update the list of connected users)\n -> 'send msg <NameUser> \"<textMsg>\"' (send a message to somebody)\n -> 'send file <NameFile> <File>' (send a file to somebody)\n\n"
#define POSSIBLE_ERR_LIST "You may want to execute the command 'list users'\nIf you need help write the command 'help'\n\n"
#define POSSIBLE_ERR_UPDATE "You may want to execute the command 'update users'\nIf you need help write the command 'help'\n\n"
#define POSSIBLE_ERR_SEND "You may want to run the command 'send msg' or 'send file'\nIf you need help write the command 'help'\n\n"
#define STOPPING_THREADS RED COLOR_BOLD"Stopping Threads currently active...\nBye!\n\n"

//Global Variables
IluvatarSonConfig *config;
int fdArda, fdIpc, fdSocket;
pthread_mutex_t mtxFdArda; //We will send arda things from different threads
struct hashmap *connectionsHashmap;
struct hashmap *threadsHashmap; //Stores all current threads in use
int run; //1 if all threads should run. 0 if they must stop
pthread_rwlock_t mtxRun; //Read-Write Lock: 1 single writer, multiple readers at once


//Function Headers
void signalHandler(int signal);
void end(int msg, int code);

void sendNewSon();
void updateUsers();
void listUsers();

void prepareSendMsg(char *buff);
void prepareSendFile(char *buff);
void checkSocketMessage();
void checkIpcMessage();
void checkArdaMessage();

#endif //EASYSTEM_ILUVATARFUNCTIONS_H
