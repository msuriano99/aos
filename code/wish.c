//
// Created by masur on 1/28/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

void handleError() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}


// Function to split the command into arguments
int splitCommand(char *command, char *args[], const char *del) {
  int i = 0;
  char *in_ptr = command;
  char *out_ptr;

  while ((out_ptr = strsep(&in_ptr, del)) != NULL) {
      args[i++] = out_ptr;
  }
  args[i] = NULL;  // Null-terminate the args array
  return i;        // Return the number of arguments
}

// Function to construct the full path and check if the file exists
int constructAndCheckPath(char *out_ptr, char *path[], char *full_path) {
  for (int idx = 0; path[idx] != NULL; ++idx) {
      snprintf(full_path, PATH_MAX, "%s/%s", path[idx], out_ptr);

      if (access(full_path, X_OK) == 0) {
          printf("File '%s' exists.\n", full_path);
          return 1;  // File found and executable
      }
  }
  return 0;  // File not found in any path
}

// Main function to check the path
void checkPath(char *args[], int *command_exists_flag, char *full_path, char *command, char *path[]) {
  const char del[2] = " ";  // Delimiter for splitting the command

  // Step 1: Split the command into arguments
  splitCommand(command, args, del);

  // Step 2: Check if the command exists in any path
  *command_exists_flag = constructAndCheckPath(args[0], path, full_path);
}

void handleCdCommand(char *args[]) {
  if (args[1] == NULL || args[2] != NULL) {
      handleError();
  } else {
      int result = chdir(args[1]);
      if (result == -1) {
          handleError();
      } else {
          char cwd[PATH_MAX];
          if (getcwd(cwd, sizeof(cwd)) != NULL) {
              printf("Directory changed to: %s\n", cwd);
          }
      }
  }
}

void handleExitCommand() {
  exit(0);
}

void handlePathCommand(char *args[], char *path[]) {
  // Clear existing path list
  for (int i = 0; i < 50; i++) {
      path[i] = NULL;
  }

  // Copy new paths from args[1] onwards
  for (int i = 0; i < 49 && args[i + 1] != NULL; i++) {
      path[i] = strdup(args[i + 1]);  // Copy string safely
  }
}

void handleExternalCommand(int command_exists_flag, char *full_path, char *args[]) {
  if (command_exists_flag == 1) {
      pid_t pid = fork();
      if (pid == 0) {
          // Child process: Execute the command
          execv(full_path, args);
          // If execv fails, exit the child process
          perror("execv failed");
          exit(1);
      } else if (pid > 0) {
          // Parent process: Wait for the child to finish
          waitpid(pid, NULL, 0);
      } else {
          // Fork failed
          perror("fork failed");
      }
  } else {
      // Command not found: Print an error
      handleError();
  }
}

void handleCommand(char *args[], char *path[], int command_exists_flag, char *full_path) {
  if (strcmp(args[0], "cd") == 0) {
      handleCdCommand(args);
  } else if (strcmp(args[0], "exit") == 0) {
      handleExitCommand();
  } else if (strcmp(args[0], "path") == 0) {
      handlePathCommand(args, path);
  } else {
      handleExternalCommand(command_exists_flag, full_path, args);
  }
}

int main() {

  char *command = NULL;
  size_t command_size = 0;


  //  Create a list to store the paths. Initially, the path should just be /bin/
  char *path[50] = {NULL};
  path[0] = "/bin";



  while (1) {
      int command_exists_flag = 0;

      printf("wish> ");
      size_t len = getline(&command, &command_size, stdin);

      // Not sure if I need this one
      if (len > 0 && command[len - 1] == '\n') {
          command[len - 1] = '\0';
      }
      printf("You entered: %s\n", command);

      //      Only the first command you check so yes we need to split it but we only check the first one.
      //Then could have redirections, multiple commands as well so need to look out for the & character and the > character

      char *args[50] = {NULL};
      char full_path[1024];  // Buffer for full path

      checkPath(args, &command_exists_flag, full_path, command, path);

      handleCommand(args, path, command_exists_flag, full_path);
  }

  free(command);
  return 0;
}

