#ifndef IPCHELPER_H
#define IPCHELPER_H

#define _GNU_SOURCE
#include <mqueue.h>
#include "connection.h"
#include "shellHelper.h"
#define DEFAULT_MSG_SIZE 512

/**
 * WORKINGS OF INTER-PROCESS COMMUNICATION
 *
 * IPC_start() will be called from each iluvatarSon when they start. A new file descriptor
 *  (mqd_t) will be created to listen for IPC requests (this message queue will have as ID
 *  the PID of the current process).
 *
 * Whenever an iluvatarSon wants to communicate with another using IPC, the first will send
 *  a message to the second's message queue (or fd, mqd_t), and the message will be a string
 *  representing the ID of the new message queue that will be created.
 *
 * For communication between these processes, the newly created queue with accorded ID will
 *  be used. The ID will be formatted as ("%d%d") (PID process1, PID process2)
 */


/**
 * Starts listening for IPC Message Queues in the POSIX queue with id=getpid(), and returns the fd
 * @return The fd of the Message Queue with id=getpid()
 */
int IPC_start();

/**
 * Stops the IPC listening and closes resources correctly
 */
void IPC_stop(int mqd);

char * IPC_sendAwakeMessage(int pid);
char * IPC_readAwakeMessage(int fd);


int* IPC_startQueues(char *id);

void IPC_stopQueues(char *id, int *mqds);

int IPC_shouldUseIPC(char *currentIp, Connection *c);

#endif
