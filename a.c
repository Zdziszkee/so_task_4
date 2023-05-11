
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {

    if (argc != 3) {
        perror("wrong number of arguments");
        exit(EXIT_FAILURE);
    }

    char* inputPath = argv[1];
    char* outputPath = argv[2];

    char buffer[100]; //buffer size =  1kB
    char termOut[1024];

    int filedes[2];
    int file, reading;

    size_t dataFrameProducer = 100; // producer max size 1000
    size_t dataFrameConsumer = 80; // consumer max size 1000

    int statLock = 0;

    /* creating a pipe */
    if (pipe(filedes) == -1) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    switch (fork()) {

        case -1:
            perror("fork error");
            exit(EXIT_FAILURE);
            /* child process */
        case 0:
            /* close pipe */
            if (close(filedes[1]) == -1) {
                perror("failed to close pipe");
                exit(EXIT_FAILURE);
            }

            /* open file */
            if ((file = open(outputPath, O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1) {
                perror("failed to open file");
                exit(EXIT_FAILURE);
            }

            while (1) {

                sleep(rand() % 5);

                /* read file */
                if ((reading = read(filedes[0], &buffer, dataFrameConsumer)) == -1) {
                    perror("failed to read file");
                    exit(EXIT_FAILURE);
                }

                if (reading == 0) break;

                /* write to file */
                if (write(file, &buffer, reading) == -1) {
                    perror("write error");
                    exit(EXIT_FAILURE);
                }

                buffer[reading] = '\0';
                sprintf(termOut, "%s%s%c", "[31mDownloading...[0m", buffer, '\n');
                write(STDOUT_FILENO, termOut, strlen(termOut));
            }

            /* close file */
            if (close(file) == -1) {
                perror("failed to close file");
                exit(EXIT_FAILURE);
            }

            /* close pipe */
            if (close(filedes[0]) == -1) {
                perror("failed to close pipe");
                exit(EXIT_FAILURE);
            }


            _exit(0);

            /* parent process */
        default:

            /* close pipe */
            if (close(filedes[0]) == -1) {
                perror("failed to close pipe");
                exit(EXIT_FAILURE);
            }

            /* open file */
            if ((file = open(inputPath, O_RDONLY, 0644)) == -1) {
                perror("failed to open file1");
                exit(EXIT_FAILURE);
            }


            while (1) {
                sleep(rand() % 5);

                /* read a file */
                if ((reading = read(file, buffer, dataFrameProducer)) == -1) {
                    perror("failed to read file");
                    exit(EXIT_FAILURE);
                }

                if (reading == 0) break;

                /* write to file */
                if (write(filedes[1], buffer, reading) == -1) {
                    perror("write error");
                    exit(EXIT_FAILURE);
                }

                /* cout */
                buffer[reading] = '\0';
                sprintf(termOut, "%s %s%c", "[32mUploading...[0m", buffer, '\n');
                write(STDOUT_FILENO, termOut, strlen(termOut));
            }

            /* close file */
            if (close(file) == -1) {
                perror("failed to close file");
                exit(EXIT_FAILURE);
            }

            /* close pipe */
            if (close(filedes[1]) == -1) {
                perror("failed to close pipe");
                exit(EXIT_FAILURE);
            }

            if (wait(&statLock) == -1) {
                perror("wait error");
                exit(EXIT_FAILURE);
            }

            break;
    }

    return 0;
}