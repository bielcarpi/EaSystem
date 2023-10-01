#include "shellHelper.h"

static char * readPipe(int fd) {
    int i = 0;
    char temp;
    char *buffer = (char *) malloc(sizeof(char));

    while(1){
        if (read(fd, &temp, sizeof(char)) < 1) break;
        buffer[i] = temp;
        buffer = (char *) realloc(buffer, sizeof(char) * (i + 2));
        i++;
    }

    if(i == 0){
        free(buffer);
        return NULL;
    }

    buffer[i - 1] = '\0';
    return buffer;
}



static int run(char *command, char **stdout, char **stderr){
    int pipeStdout[2], pipeStderr[2];
    pipe(pipeStdout);
    pipe(pipeStderr);

    pid_t child = fork();
    if(child == 0){
        //Close reading ends in the child
        close(pipeStdout[0]);
        close(pipeStderr[0]);

        //Send STDERR & STDOUT to the pipes
        dup2(pipeStdout[1], STDOUT_FILENO);
        dup2(pipeStderr[1], STDERR_FILENO);

        //Close no longer needed descriptors
        close(pipeStdout[1]);
        close(pipeStderr[1]);

        char *arr[] = {"/bin/sh", "-c", command, NULL};
        execv("/bin/sh", arr);
    }
    else if(child == -1){
        *stdout = NULL;
        *stderr = NULL;
        return -1;
    }
    else{
        //Close the writing ends of the pipes in the parent
        close(pipeStdout[1]);
        close(pipeStderr[1]);

        int status;
        wait(&status); //Let's wait until the child process is done
        int returned = WEXITSTATUS(status);

        *stdout = readPipe(pipeStdout[0]);
        *stderr = readPipe(pipeStderr[0]);

        //Close the reading ends of the pipes. We're done
        close(pipeStdout[0]);
        close(pipeStderr[0]);

        return returned;
    }

    return -1;
}


void SHH_runCommand(char *command){
    char *stdout, *stderr;
    int code = run(command, &stdout, &stderr);

    switch(code){
        case -1:
            INOUT_print(ERR_FORK);
            break;
        case 127:
            INOUT_print(ERR_UNKNOWN_COMMAND);
            break;
        case 0:
            if(stdout != NULL) INOUT_print(stdout);
            else if(stderr != NULL) INOUT_print(stderr);
            INOUT_print("\n\n");
            break;
        default:
            if(stderr != NULL) INOUT_print(stderr);
            else if(stdout != NULL) INOUT_print(stdout);
            else INOUT_print(ERR_COMMAND);
            INOUT_print("\n\n");
    }

    if(stdout != NULL) free(stdout);
    if(stderr != NULL) free(stderr);
}

char * SHH_getDomain(char *ip){
    char *command;
    asprintf(&command, "dig -x %s +short", ip);

    char *stdout, *stderr;
    int code = run(command, &stdout, &stderr);
    free(command);
    if(stderr != NULL) free(stderr);

    switch(code){
        case 0:
            if(stdout != NULL) stdout[strlen(stdout) - 1] = '\0'; //dig ends name with a '.'
            return stdout;
        default:
            if(stdout != NULL) free(stdout);
            return NULL;
    }
}


char * SHH_getHashMD5(char* fileName){
    char *command;
    asprintf(&command, "md5sum %s | awk '{ print $1 }'", fileName);
    char *stdout, *stderr;
    int code = run(command, &stdout, &stderr);
    free(command);
    if(stderr != NULL) free(stderr);

    switch(code){
        case 0:
            if(stdout != NULL) stdout[strlen(stdout) - 1] = '\0';
            return stdout;
        default:
            if(stdout != NULL) free(stdout);
            return NULL;
    }
}

int SHH_processExists(int pid){
    char *command;
    asprintf(&command, "ps -u $(whoami) | grep %d", pid);
    char *stdout, *stderr;
    int code = run(command, &stdout, &stderr);
    free(command);
    if(stderr != NULL) free(stderr);

    if(code == 0 && stdout != NULL && strlen(stdout) > 5){
        free(stdout);
        return 1; //Yes, the process exists
    }

    if(stdout != NULL) free(stdout);
    return 0; //No, the process doesn't exist, or we can't check for its existance
}

char * SHH_getBasename(char *filePath){
    char *command;
    asprintf(&command, "basename %s", filePath);
    char *stdout, *stderr;
    int code = run(command, &stdout, &stderr);
    free(command);
    if(stderr != NULL) free(stderr);

    if(code == 0) return stdout;

    if(stdout != NULL) free(stdout);

    char *empty = (char *) malloc(sizeof(char));
    empty[0] = 0;
    return empty;
}
