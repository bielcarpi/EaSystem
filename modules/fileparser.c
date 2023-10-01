#include "fileparser.h"

IluvatarSonConfig *FP_parseIluvatarSonConfig(const char *path) {
    int fdIluvatarSonConfig = open(path, O_RDONLY);
    if (fdIluvatarSonConfig < 0)
        return NULL;

    IluvatarSonConfig *iluvatarSonConfig = (IluvatarSonConfig *) malloc(sizeof(IluvatarSonConfig));
    if (iluvatarSonConfig == NULL)
        return NULL;

    char *buffer = (char *) malloc(sizeof(char));
    char nextChar;
    int count = 0;
    int field = 0;
    while (read(fdIluvatarSonConfig, &nextChar, sizeof(char)) != 0) {
        if (nextChar == '\n') {
            buffer[count] = '\0';
            switch (field) {
                case 0:
                    iluvatarSonConfig->name = buffer;
                    break;
                case 1:
                    iluvatarSonConfig->folder = buffer;
                    break;
                case 2:
                    iluvatarSonConfig->ipArda = buffer;
                    break;
                case 3:
                    iluvatarSonConfig->portArda = buffer;
                    break;
                case 4:
                    iluvatarSonConfig->ip = buffer;
                    break;
                case 5:
                    iluvatarSonConfig->port = buffer;
                    break;
            }
            buffer = (char *) malloc(sizeof(char));
            count = 0;
            field++;
        }
        else if(nextChar == '&'){
            continue;
        }
        else {
            buffer[count] = nextChar;
            count++;
            buffer = (char *) realloc(buffer, sizeof(char) * (count + 1));
        }
    }

    close(fdIluvatarSonConfig);
    free(buffer);
    return iluvatarSonConfig;
}

void FP_freeIluvatarSonConfig(IluvatarSonConfig *config) {
    if (config != NULL) { //If the config pointer is not null, free its contents
        free(config->name);
        free(config->folder);
        free(config->ipArda);
        free(config->portArda);
        free(config->ip);
        free(config->port);
        free(config);
    }
}


ArdaConfig *FP_parseArdaConfig(const char *path) {
    int fdArdaConfig = open(path, O_RDONLY);
    if (fdArdaConfig < 0)
        return NULL;

    ArdaConfig *ardaConfig = (ArdaConfig *) malloc(sizeof(ArdaConfig));
    if (ardaConfig == NULL)
        return NULL;

    char *buffer = (char *) malloc(sizeof(char));
    char nextChar;
    int count = 0;
    int field = 0;
    while (read(fdArdaConfig, &nextChar, sizeof(char)) != 0) {
        if (nextChar == '\n') {
            buffer[count] = '\0';
            switch (field) {
                case 0:
                    ardaConfig->ip = buffer;
                    break;
                case 1:
                    ardaConfig->port = buffer;
                    break;
                case 2:
                    ardaConfig->directory = buffer;
                    break;
            }
            buffer = (char *) malloc(sizeof(char));
            count = 0;
            field++;
        }
        else if(nextChar == '&'){
            continue;
        }
        else {
            buffer[count] = nextChar;
            count++;
            buffer = (char *) realloc(buffer, sizeof(char) * (count + 1));
        }
    }

    close(fdArdaConfig);
    free(buffer);
    return ardaConfig;
}

void FP_freeArdaConfig(ArdaConfig *config) {
    if (config != NULL) { //If the config pointer is not null, free its contents
        if(config->ip != NULL) free(config->ip);
        if(config->port != NULL) free(config->port);
        if(config->directory != NULL) free(config->directory);
        free(config);
    }
}
