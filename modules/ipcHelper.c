#include "ipcHelper.h"


int IPC_start(){
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = DEFAULT_MSG_SIZE;
    attr.mq_curmsgs = 0;

    char *pid;
    asprintf(&pid, "/%d", getpid());
    int IPCqueue = mq_open(pid, O_CREAT | O_RDONLY, 0644, &attr);
    free(pid);

    return IPCqueue;
}


void IPC_stop(int mqd){
    mq_close(mqd);

    char *pid;
    asprintf(&pid, "/%d", getpid());
    mq_unlink(pid);
    free(pid);
}


char * IPC_sendAwakeMessage(int pid){
    char *buff;
    asprintf(&buff, "/%d", pid);
    int IPCqueue = mq_open(buff, O_WRONLY);
    free(buff);
    if(IPCqueue < 0) return NULL;

    buff = (char *) malloc(sizeof(char) * DEFAULT_MSG_SIZE);
    memset(buff, 0, DEFAULT_MSG_SIZE);

    srand(time(NULL));
    sprintf(buff, "%d%d_%d", pid, getpid(), rand());
    mq_send(IPCqueue, buff, DEFAULT_MSG_SIZE, 0);
    mq_close(IPCqueue);

    return buff;
}

char * IPC_readAwakeMessage(int fd){
    char *buff = (char *) malloc(DEFAULT_MSG_SIZE);
    mq_receive(fd, buff, DEFAULT_MSG_SIZE, NULL);
    return buff;
}



int * IPC_startQueues(char *id){
    int *queues = (int *) malloc(sizeof(int) * 2);
    char *idRead, *idWrite;
    asprintf(&idRead, "/%s_rw", id);
    asprintf(&idWrite, "/%s_wr", id);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = DEFAULT_MSG_SIZE;
    attr.mq_curmsgs = 0;

    queues[0] = mq_open(idRead, O_CREAT | O_RDWR, 0644, &attr);
    queues[1] = mq_open(idWrite, O_CREAT | O_RDWR, 0644, &attr);

    free(idRead);
    free(idWrite);

    return queues;
}


void IPC_stopQueues(char *id, int *mqds){
    char *idRead, *idWrite;
    asprintf(&idRead, "/%s_rw", id);
    asprintf(&idWrite, "/%s_wr", id);

    mq_close(mqds[0]);
    mq_close(mqds[1]);
    mq_unlink(idRead);
    mq_unlink(idWrite);

    free(idRead);
    free(idWrite);
    free(mqds);
    free(id);
}


int IPC_shouldUseIPC(char *currentIp, Connection *c){
    if(c == NULL) return 0;

    if(strcmp(c->ip, currentIp) != 0) return 0;
    return SHH_processExists((int) c->pid);
}