#ifndef SOCKETHELPER_H
#define SOCKETHELPER_H

#define _GNU_SOURCE
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "stdinout.h"

#define ERR_CRT_SOCKET "\n\x1B[31mError creating the socket\n\n\x1B[0m"
#define ERR_CONF_IP "\n\x1B[31mError configuring the IP\n\n\x1B[0m"
#define ERR_BIND "\n\x1B[31mError with the BIND function\n\n\x1B[0m"
#define ERR_LIST "\n\x1B[31mError Listening the Socket\n\n\x1B[0m"
#define ERR_CONN "\x1B[31mError Connecting to desired socket.\n\n\x1B[0m"
#define LISTENING_QUEUE 15

/**
 * Starts listening for connections in TCP socket ip:port provided, and returns the fd of the data stream
 * @param ip The ip of the socket
 * @param port The port of the socket
 * @return The file descriptor of the TCP socket listening for connections
 */
int SCK_startServer(char *ip, int port);

/**
 * Tries to connect to a TCP socket with ip:port provided, returning the fd of the data stream
 * @param ip The ip of the socket we want to connect to
 * @param port The port of the socket we want to connect to
 * @return The file descriptor of the opened TCP socket
 */
int SCK_connectToServer(char *ip, int port);

#endif