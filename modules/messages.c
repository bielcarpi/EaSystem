#include "messages.h"

void * FP_parseMsgFile(char *buff) {
    int countSpace = 0;
    char *secondPartFunction;
    char *buffNom;
    char *token;
    token = strtok(buff , " ");
    while(token != NULL){
        switch (countSpace) {
            case 1:
                secondPartFunction = token;
                break;
            case 2:
                buffNom = strdup(token);
                break;
        }
        countSpace++;
        if(countSpace < 3){
            token = strtok(NULL, " ");
        }
        else if(countSpace == 3 && strcmp(secondPartFunction, "msg") == 0){
            token = strtok(NULL, """");

            if(token != NULL){
                Message *m = (Message*) malloc(sizeof(Message));
                m->originUser = buffNom;
                m->message = strdup(token);
                if(m->message[strlen(m->message) - 1] == '"')
                    return m;
                else{
                    MS_freeMessage(m);
                    return NULL;
                }
            }

            free(buffNom);
            return NULL;

        }
        else if(countSpace == 3 && strcmp(secondPartFunction, "file") == 0){
            token = strtok(NULL, " ");

            if(token != NULL) {
                File *f = (File*) malloc(sizeof(File));
                f->originUser = buffNom;
                f->fileName = strdup(token);
                f->fileSize = NULL;
                f->hashDM5 = NULL;
                return f;
            }

            free(buffNom);
            return NULL;
        }
    }

    return NULL;
}


Message * MS_deserializeMessage(const char *data){
    Message* m = (Message*) malloc(sizeof(Message));
    int parm = sscanf(data, "%m[^&]&%m[^\n]", &(m->originUser), &(m->message));
    if(parm != 2){
        free(m);
        return NULL;
    }

    return m;
}

char* MS_serializeMessage(Message* message){
    char* data;
    asprintf(&data, "%s&%s", message->originUser, message->message);
    return data;
}

void MS_freeMessage(Message *message){
    if(message != NULL){
        if(message->originUser != NULL) free(message->originUser);
        if(message->message != NULL) free(message->message);
        free(message);
    }
}


char * MS_serializeFile(File *file){
    char* data;
    asprintf(&data,"%s&%s&%s&%s",file->originUser, file->fileName, file->fileSize, file->hashDM5);
    return data;
}



File * MS_deserializeFile(const char *data) {
    File *f = (File *) malloc(sizeof(File));
    int param = sscanf(data, "%m[^&]&%m[^&]&%m[^&]&%ms", &(f->originUser), &(f->fileName), &(f->fileSize), &(f->hashDM5));
    if (param != 4) {
        free(f);
        return NULL;
    }
    return f;
}


void MS_freeFile(File* file){
    if(file != NULL){
        if(file->originUser != NULL) free(file->originUser);
        if(file->fileName != NULL) free(file->fileName);
        if(file->fileSize != NULL) free(file->fileSize);
        if(file->hashDM5 != NULL) free(file->hashDM5);
        free(file);
    }
}




