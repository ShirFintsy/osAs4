// Shir Fintsy 206949075

#include<stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <string.h>

/*
 * exits the program if 60 seconds past since the program started or the last client apply
 */
void timer() {
    printf("The server was closed because no service request was received for the last 60 seconds\n");
    exit(0);
}

char** split(char* string) {
    char** stringArray;
    // Extract the first token
    char * token = strtok(string, " ");
    int i = 0;
    // loop through the string to extract all other tokens
    while( token != NULL ) {
        //printf( " %s\n", token ); //printing each token
        stringArray[i] = token;
        token = strtok(NULL, " ");
        i++;
    } return stringArray;
}

void createFile(char* pid, int result, int zeroFlag) { // not working
    char pidString[] = "to_client_";
    char copy[10000];
    strcpy(copy, pid); // convert from char* to char[]
    strcat(pidString,  copy);
    strcat(pidString, ".txt");
    FILE * toClient;
    // create the "to_client_XXX" file
    toClient = fopen(pidString, "w");
    //printf("file createdi in server: %s\n", pidString); // debug
    //write to file "to_client_xxx" the result
    char sResult[1000];
    sprintf(sResult, "%d", result); //convert int to char*
    if (zeroFlag == 0) {
        fputs(sResult, toClient);
    } else
        fputs("zero", toClient);
    // close file
    fclose(toClient);
}

void handleClient(int signal) {
    int num1, num2, operator;
    char* clientPid;
    char** params;
    alarm(60); // notify a new client sent a signal
    //1. when got signal - create new process
    int stat = 0;
    pid_t pid = fork();

    if (pid == -1) { // the fork failed
        printf("ERROR_FROM_EX4\n");
    } else if (pid == 0) { // the child
        //2. read from file "to_srv" the information
        char * buffer = 0;
        long length;
        FILE * f = fopen ("to_srv.txt", "rb");
        if (f)
        {
            fseek (f, 0, SEEK_END);
            length = ftell (f);
            fseek (f, 0, SEEK_SET);
            buffer = malloc (length);
            if (buffer)
            {
                fread (buffer, 1, length, f);
            }
            fclose (f);
            params =  split(buffer);
        } else // failed to open file "to_srv.txt"
            printf("ERROR_FROM_EX4\n");
        //3. delete the file "to_srv.txt"
        if (remove("to_srv.txt") != 0)  // failed to delete file
            printf("ERROR_FROM_EX4\n");
        // else file was deleted
        // set parameters:
        clientPid = params[0];
        num1 = strtol(params[1], NULL, 10); // convert from char* to int
        operator = strtol(params[2], NULL, 10);
        num2 = strtol(params[3], NULL, 10);
        //4. calculate
        int result = -1, zeroFlag = 0;
        switch (operator) {
            case(1): {
                result = num1 + num2;
                break;
            }
            case(2): {
                result = num1 - num2;
                break;
            }
            case(3): {
                result = num1 * num2;
                break;
            }
            case(4): {
                if (num2 != 0) {
                    result = num1 / num2;
                } else
                    zeroFlag = 1;
                break;
            }
            default: { // not valid operator
                printf("ERROR_FROM_EX4\n");
                break;
            }
        }
        //5. create file to the client
        createFile(clientPid, result, zeroFlag);

        //7. send signal to client
        pid_t cPid = strtol(clientPid, NULL, 10);
        
        kill(cPid, SIGINT);
    } else {
        // waiting for child to terminate
        wait(&stat);
    }

}

int main(int argv, char* argc[]) {
    signal(SIGINT, handleClient);
    signal(SIGALRM, timer);
    alarm(60); // timer
    //handleClient(0); debug
    FILE *file;
    char* fileName = "to_srv.txt";
    file = fopen(fileName, "r");

    //1. delete file if exists
    if (file) {// file exists
        fclose(file);
        // delete file:
        if (remove(fileName) != 0)  // delete file failed
            printf("ERROR_FROM_EX4\n");
        // else file was deleted
    }

    //2. wait for signals
    while(1) {
        pause();
    }
}