#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// This is the maximum number of arguments your shell should handle for one command
#define MAX_ARGS 128

int main(int argc, char** argv) {
  // If there was a command line option passed in, use that file instead of stdin
  if (argc == 2) {
    // Try to open the file
    int new_input = open(argv[1], O_RDONLY);
    if (new_input == -1) {
      fprintf(stderr, "Failed to open input file %s\n", argv[1]);
      exit(1);
    }

    // Now swap this file in and use it as stdin
    if (dup2(new_input, STDIN_FILENO) == -1) {
      fprintf(stderr, "Failed to set new file as input\n");
      exit(2);
    }
  }

  char* line = NULL;     // Pointer that will hold the line we read in
  size_t line_size = 0;  // The number of bytes available in line

  // Loop forever
  while (true) {
    // Print the shell prompt
    printf("$ ");

    // Get a line of stdin, storing the string pointer in line
    if (getline(&line, &line_size, stdin) == -1) {
      if (errno == EINVAL) {
        perror("Unable to read command line");
        exit(2);
      } else {
        // Must have been end of file (ctrl+D)
        printf("\nShutting down...\n");

        // Exit the infinite loop
        break;
      }
    }

    // TODO: Execute the command instead of printing it below
    // printf("Received command: %s\n", line);

    // Declaring variables for moving through line and storing child processes
    char* sep;
    char* str = line;
    pid_t childArr[MAX_ARGS];
    int childArrLen = 0;

    // Loops through the input until user enters "exit" or until all        commands in the input
    // have been run
    while (str != NULL) {
      if (strcmp(str, "\n") == 0) break;

      // Breaks str up at space where one of the delimiters is found

      sep = strpbrk(str, ";&\n");

      if (sep == NULL) {
        break;
      }

      // Handles the case where the '&' delimiter is found

      if (*sep == '&') {
        *sep = '\0';
        if (sep + 1 == NULL) break;

        // Breaks down line into the command and its parameters, then formats the given array

        char* lineArr[MAX_ARGS];
        int lenLineArr = 0;
        for (int i = 0; i < MAX_ARGS; i++) {
          lineArr[i] = strsep(&str, " ");
          if (lineArr[i] == NULL) {
            break;
          }
          lenLineArr++;
        }

        str = sep + 1;

        lineArr[lenLineArr - 1] = strsep(&lineArr[lenLineArr - 1], "\n");

        int numSpace = 0;
        for (int i = 0; i < lenLineArr; i++) {
          if (strcmp(lineArr[i], "") == 0) {
            numSpace++;
            for (int j = i; j < lenLineArr - 1; j++) {
              lineArr[j] = lineArr[j + 1];
            }
          }
        }
        for (int i = 0; i < lenLineArr; i++) {
          if (strcmp(lineArr[i], "") == 0) lineArr[i] = NULL;
        }

        // Forks to run process in background

        int childPID = fork();
        childArr[childArrLen] = childPID;
        childArrLen++;
        if (childPID == -1) {
          perror("Fork error");
          exit(EXIT_FAILURE);
        }
        if (childPID == 0) {
          char* cmd = lineArr[0];
          if (execvp(cmd, lineArr) == -1) {
            perror("Execvp error");
            exit(EXIT_FAILURE);
          }
        }
      }

      // Handles the cases where the delimiter is not '&'

      else {
        *sep = '\0';

        // Creates array of command and parameters in line and formats

        char* lineArr[MAX_ARGS];
        int lenLineArr = 0;
        for (int i = 0; i < MAX_ARGS; i++) {
          lineArr[i] = strsep(&str, " ");
          if (lineArr[i] == NULL) {
            break;
          }
          lenLineArr++;
        }

        str = sep + 1;

        lineArr[lenLineArr - 1] = strsep(&lineArr[lenLineArr - 1], "\n");

        int numSpace = 0;
        for (int i = 0; i < lenLineArr; i++) {
          if (strcmp(lineArr[i], "") == 0) {
            numSpace++;
            for (int j = i; j < lenLineArr - 1; j++) {
              lineArr[j] = lineArr[j + 1];
            }
          }
        }
        lineArr[lenLineArr - numSpace] = NULL;

        // Changes diectory, exites, runs command, or does nothing based on lineArr[0]

        if (strcmp(lineArr[0], "cd") == 0) {
          chdir(lineArr[1]);
        } else if (strcmp(lineArr[0], "") == 0) {
        } else if (strcmp(lineArr[0], "exit") == 0) {
          exit(0);
        } else {
          int childPID = fork();

          // Checks for possible errors and if none, runs command in child process

          if (childPID == -1) {
            perror("Fork error");
            exit(EXIT_FAILURE);
          }

          if (childPID == 0) {
            char* cmd = lineArr[0];
            if (execvp(cmd, lineArr) == -1) {
              perror("Execvp error");
              exit(EXIT_FAILURE);
            }
          }

          // Setting up and printing exit status

          else {
            int status;
            pid_t result = wait(&status);
            int wstatus = WEXITSTATUS(result);
            if (status == -1) {
              perror("Wait error");
              exit(EXIT_FAILURE);
            }

            printf("[%s exited with status %d]\n", lineArr[0], wstatus);
          }
        }
      }
    }

    // Checks for exiting background processes and prints exit status

    for (int i = 0; i < childArrLen; i++) {
      int status;
      waitpid(childArr[i], &status, WNOHANG);
      printf("[background process %d exited with status %d]\n", childArr[i], status);
    }
  }

  // If we read in at least one line, free this space
  if (line != NULL) {
    free(line);
  }

  return 0;
}
