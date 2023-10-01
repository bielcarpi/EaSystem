#ifndef EASYSTEM_ARDAFUNCTIONS_H
#define EASYSTEM_ARDAFUNCTIONS_H

#define _GNU_SOURCE
#include <signal.h>
#include "fileparser.h"
#include "communications.h"
#include "connection.h"
#include "socketHelper.h"
#include "stdinout.h"

#define ERR_ARGS RED"\nIntroduce a valid number of arguments\n"
#define ERR_ACCEPT RED"\nError Accepting a Connection\n"
#define ERR_USER_NAME RED"Error. The username trying to connect to the server already exists. Disconnected.\n\n"
#define ERR_WRITE RED"\nError sending the packet. Disconnecting from this user.\n\n"
#define EXIT_SIGNAL RED COLOR_BOLD"\nTerminating due to signal: %s\n\n"
#define SERVER_FULL RED COLOR_BOLD"\nThe server has reached MAX_CLIENTS and someone is trying to connect. Waiting for disconnections...\n"
#define STOP_SUCCESS CYN COLOR_BOLD"\nTotal num of messages sent through the network: %d\n\x1B[31mEverything has been stopped & closed successfully. Arda has been shut down.\n\n"
#define SERVER_START GRN"\nArda Server Started\n"
#define READING_FILES "Reading configuration file\n"
#define WAITING_CONNECTIONS "Waiting connections...\n\n"
#define NEW_LOGIN YEL COLOR_BOLD"New Login: %s, IP: %s, port: %hu, PID %d, FD: %d\n"
#define DISCONNECTING_OLD_SESSION BLU COLOR_BOLD"Disconnecting Old session of the same User, Ip & Port...\n"
#define USER_LIST_REQUEST "Updating user’s list\nSending user’s list\n"
#define USER_LIST_REQUEST_UPDATE YEL COLOR_BOLD"New Petition: fd %d (%s) demands the user's list\n"
#define UNKNOWN_REQUEST "Unknown request received by Connection with fd %d\n"
#define DISCONNECTED RED"User %s, IP: %s, port: %hu, PID: %d, FD: %d has been disconnected\n\n"
#define RESPONSE_SENT "Response Sent\n\n"
#define RESPONSE_SENT_UPDATE "Sending user's list to %s\n\n"

#define MAX_CLIENTS 1000 //Select & fd_set restriction. We could use poll/epoll for more clients
#define MESSAGES_COUNT_FILE_NAME "resources/messagesCount.bin"

//Global variables
fd_set fdset;
int fdListen, biggerFd;
ArdaConfig *conf;
struct hashmap *connectionsHashmap;
int numMessages;
char *connectionsList; //A cached version of all the current connections


//Function Headers

void handleNewConnection();
void handleRequest(int fd);
void signalHandler(int signal);
void stop(int exitCode);


void handleListRequest(int fd, int newConnection);
void handleUnknownRequest(int fd);
void disconnect(int fd);

#endif
//EASYSTEM_ARDAFUNCTIONS_H