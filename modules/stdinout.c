#include "stdinout.h"
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void INOUT_readStdin(char* *formattedBuff,char* *buff){
    char nextChar;
    int count = 0;
    *buff = (char *) malloc(sizeof(char));
    *formattedBuff = NULL;
    while (1) {
        read(STDIN_FILENO, &nextChar, sizeof(char));
        if (nextChar == '\n') {
            (*buff)[count] = '\0';
            count = 0;
            for(size_t i = 0, j = 0; i < strlen(*buff); i++){
                char c = tolower((*buff)[i]);
                if(c != ' '){
                    if(*formattedBuff == NULL)  *formattedBuff = (char *) malloc(sizeof(char));
                    else *formattedBuff = (char *) realloc(*formattedBuff, sizeof(char) * (j + 1));
                    (*formattedBuff)[j] = c;
                    j++;
                    count++;
                }
            }
            *formattedBuff = (char *) realloc(*formattedBuff, sizeof(char) * (count + 1));
            (*formattedBuff)[count] = '\0';
            break;
        }else{
            (*buff)[count] = nextChar;
            count++;
            *buff = (char *) realloc(*buff, sizeof(char) * (count + 1));
        }
    }
}



void INOUT_print(char* buff){
    pthread_mutex_lock(&mtx);
    write(STDOUT_FILENO, RESET, sizeof(char) * strlen(RESET));
    write(STDOUT_FILENO, buff, sizeof(char) * strlen(buff));
    pthread_mutex_unlock(&mtx);
}

void INOUT_end(){
    pthread_mutex_destroy(&mtx);
}