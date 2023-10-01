#include "connection.h"


int CON_connectionCompareFDs(const void *a, const void *b, void *udata) {
    const Connection *ca = a;
    const Connection *cb = b;
    return ca->fd != cb->fd; //0 true, else false
}


int CON_connectionCompareNames(const void *a, const void *b, void *udata) {
    const Connection *ca = a;
    const Connection *cb = b;
    return strcmp(ca->userName, cb->userName);
}


uint64_t CON_connectionHashByFDs(const void *item, uint64_t seed0, uint64_t seed1) {
    char *buff;
    asprintf(&buff, "%d", ((Connection *)item)->fd);
    uint64_t sip = hashmap_sip(buff, strlen(buff), seed0, seed1);
    free(buff);
    return sip;
}


uint64_t CON_connectionHashByNames(const void *item, uint64_t seed0, uint64_t seed1) {
    const Connection *con = item;
    return hashmap_sip(con->userName, strlen(con->userName), seed0, seed1);
}


void CON_freeConnection(void* item) {
    Connection *connection = (Connection *) item;
    if(connection == NULL)
        return;

    if(connection->userName != NULL)
        free(connection->userName);
    if(connection->ip != NULL)
        free(connection->ip);
    if(connection->domainName != NULL)
        free(connection->domainName);
}


Connection * CON_getConnectionByName(struct hashmap *map, char *name){
    size_t iter = 0;
    void *item;
    while (hashmap_iter(map, &iter, &item)) {
        Connection *con = item;
        if(strcmp(con->userName, name) == 0) return con;
    }

    return NULL;
}


void CON_getConnectionsList(struct hashmap *map, int *numConnections, Connection ***connections){
    *connections = (Connection **) malloc(sizeof(Connection *));
    size_t iter = 0, i = 0;
    void *item;
    while (hashmap_iter(map, &iter, &item)) {
        Connection *con = item;
        if(i != 0) *connections = (Connection **) realloc(*connections, sizeof(Connection *) * (i+1));
        (*connections)[i] = con;
        i++;
    }

    *numConnections = (int) i;
}

void CON_deserializeConnections(const char *data, int *numConnections, Connection ***connections){
    *numConnections = 0;
    *connections = NULL;
    char *token;
    char *dataBuff = strdup(data);

    token = strtok(dataBuff, "#");
    if(token == NULL)
        return;

    *connections = (Connection **) malloc(sizeof(Connection *));

    while(token != NULL){
        (*connections)[*numConnections] = CON_deserializeConnection(token);
        token = strtok(NULL, "#");
        (*numConnections)++;
        if(token != NULL) *connections = (Connection **) realloc(*connections, sizeof(Connection *) * ((*numConnections)+1));
    }

    free(dataBuff);
}

Connection * CON_deserializeConnection(const char *data) {
    Connection *c = (Connection *) malloc(sizeof(Connection));
    memset(c, 0, sizeof(Connection));
    int i = sscanf(data, "%m[^&]&%m[^&]&%hd&%d", &(c->userName), &(c->ip), &(c->port), &(c->pid));
    if(i != 4){
        free(c);
        return NULL;
    }

    return c;
}


char * CON_serializeConnections(int numConnections, Connection **connections){
    char* buff;
    char* data = (char *) malloc(sizeof(char));
    data[0] = '\0';

    for(int i = 0; i < numConnections; i++){
        buff = CON_serializeConnection(connections[i]);
        data = (char *) realloc(data, sizeof(char) * (strlen(data) + strlen(buff) + 1));
        strcat(data, buff);
        free(buff);

        if(i != (numConnections-1)){
            data = (char *) realloc(data, sizeof(char) * ((strlen(data) + 2)));
            strcat(data, "#");
        }
    }

    return data;
}

char * CON_serializeConnection(Connection *connection){
    char *bufferData;
    asprintf(&bufferData, "%s&%s&%d&%d", connection->userName, connection->ip, connection->port, connection->pid);
    return bufferData;
}
