// Shir Fintsy 206949075

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/random.h>
#include <signal.h>
#include <ctype.h>

int isNumber(char s[])
{
    for (int i = 0; s[i]!= '\0'; i++)
    {
        if (isdigit(s[i]) == 0)
            return 0;
    }
    return 1;
}

void answerFromSrv(int signal) {
    pid_t clientPid = getpid();
    char * buffer = 0;
    long length;
    //1. open the file and read from it
    char pidString[] = "to_client_";
    char sPid[10000], copy[10000];
    sprintf(sPid, "%d", clientPid); //convert int to char*
    strcpy(copy, sPid); // convert from char* to char[]
    strcat(pidString,  copy);
    strcat(pidString, ".txt");

    FILE * f = fopen (pidString, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length);
        if (buffer) {
            fread(buffer, 1, length, f);
        }
        fclose(f);
        // delete the file - no more needed
        if (remove(pidString) != 0)  // delete file failed
            printf("ERROR_FROM_EX4\n");
        // else file was deleted
    }
    //2. print to screen the result
    if (!isNumber(buffer)) { // divided by zero case
        printf("CANNOT_DIVIDE_BY_ZERO\n");
        return;
    }
    printf("%s\n", buffer);
}

void handleSrvFile(char num1[], char operator[], char num2[]) {
    pid_t clientPid = getpid();
    char cPidS[1000];
    sprintf(cPidS, "%d", clientPid); // convert from int to char[]
    FILE *file;
    char* fileName = "to_srv.txt";
    int i = 0;
    while(i < 10) {
        if (access( fileName, F_OK ) == 0) {// file exists
            //timer:
            int t;
            getrandom(&t, sizeof(int), 0);
            t = abs(t % 6); // random number between 0 and 5
            //printf("[%d] random number: %d\n",i, t);
            sleep(t);
            i++;
        } else { // file does not exist
            file = fopen(fileName, "w");
            strcat(strcat(cPidS, " "), strcat(strcat(num1, " "), strcat(strcat(operator, " "), num2)));
            //printf("string of content of file in client is: %s\n", cPidS);
            //4. write in file the information
            fputs(cPidS, file);
            fclose(file);
            return;
        }
    }
    printf("ERROR_FROM_EX4\n");
    exit(0);
}

void noAnswer() {
    printf("Client closed because no response was received from the server for 30 seconds\n");
    FILE *file;
    char* fileName = "to_srv.txt";
    file = fopen(fileName, "r");

    // delete file if exists
    if (file) {// file exists
        fclose(file);
        // delete file:
        if (remove(fileName) != 0)  // delete file failed
            printf("ERROR_FROM_EX4\n");
        // else file was deleted
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, answerFromSrv);
    signal(SIGALRM, noAnswer);

    //1. check validation of arguments
    if (argc < 5) {
        printf("ERROR_FROM_EX4\n");
        return 0;
    } else { //  check validation
        for (int i = 1; i < 5; ++i) {
            if (!isNumber(argv[i])){
                printf("ERROR_FROM_EX4\n"); // arguments are not numbers
                return 0;
            }
        }
    }
    //2. save arguments as variables and get
    int srvPid = strtol(argv[1], NULL, 10);
    int num1 = strtol(argv[2], NULL, 10);
    int operator = strtol(argv[3], NULL, 10);
    // check if operator is valid:
    if (operator > 4 || operator < 1) {
        printf("ERROR_FROM_EX4\n");
        return 0;
    }
    int num2 = strtol(argv[4], NULL, 10);
    // convert int to char[]:
    char num1S[1000], num2S[1000], operatorS[10];
    sprintf(num1S, "%d", num1);
    sprintf(num2S, "%d", num2);
    sprintf(operatorS, "%d", operator);
    //3. try to open "to_srv" file
    handleSrvFile( num1S, operatorS, num2S);
    //5. send signal to server
    kill(srvPid, SIGINT);

    //6. timer to get signal from server
    alarm(30);
    //7. if got signal - print the result (answerFromSrv)
    pause();
}