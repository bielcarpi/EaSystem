#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>

typedef struct {
    char *name;
    char *folder;
    char *ipArda;
    char *portArda;
    char *ip;
    char *port;
} IluvatarSonConfig;

typedef struct {
    char *ip;
    char *port;
    char *directory;
} ArdaConfig;


/**
 * Reads and parses an IluvatarSon config file, and returns it
 * @param path The path of the IluvatarSon config file to read
 * @return The IluvatarSonConfig. Or NULL if something has occurred
 */
IluvatarSonConfig * FP_parseIluvatarSonConfig(const char *path);

/**
 * Frees an IluvatarSonConfig struct
 * @param config The struct to be freed
 */
void FP_freeIluvatarSonConfig(IluvatarSonConfig *config);

/**
 * Reads and parses an Arda config file, and returns it
 * @param path The path of the Arda config file to read
 * @return The ArdaConfig. Or NULL if something has occurred
 */
ArdaConfig * FP_parseArdaConfig(const char *path);

/**
 * Frees an ArdaConfig struct
 * @param config The struct to be freed
 */
void FP_freeArdaConfig(ArdaConfig *config);


#endif
