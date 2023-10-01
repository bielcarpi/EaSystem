#ifndef CONNECTION_H
#define CONNECTION_H

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hashmap.h"

typedef struct {
    char *userName;
    char *ip;
    uint16_t port;
    uint32_t pid;
    uint16_t fd; //optional
    char *domainName; //optional
} Connection;


int CON_connectionCompareFDs(const void *a, const void *b, void *udata);
int CON_connectionCompareNames(const void *a, const void *b, void *udata);
uint64_t CON_connectionHashByFDs(const void *item, uint64_t seed0, uint64_t seed1);
void CON_freeConnection(void *item);
uint64_t CON_connectionHashByNames(const void *item, uint64_t seed0, uint64_t seed1);
Connection * CON_getConnectionByName(struct hashmap *map, char *name);
void CON_getConnectionsList(struct hashmap *map, int *numConnections, Connection ***connections);

void CON_deserializeConnections(const char *data, int *numConnections, Connection ***connections);
Connection * CON_deserializeConnection(const char *data);
char * CON_serializeConnections(int numConnections, Connection **connections);
char * CON_serializeConnection(Connection *connection);

#endif