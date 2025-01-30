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
#include <ctype.h>

// NEED TO ADD CONSTANTS INSTEAD OF HARD CODED VALUES

void handleError() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
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
  //printf("%d\n", 1);
  //printf("Command FLAG: %d\n", command_exists_flag);
  if (command_exists_flag == 1) {
      pid_t pid = fork();
      if (pid == 0) {
          //printf("FULL PATH: %s\n", full_path);
          for (int i = 0; args[i] != NULL; i++) {
              printf("%s", args[i]);
          }

          // Child process: Execute the command
          execv(full_path, args);
          // If execv fails, exit the child process
          perror("execv failed");
          exit(1);
      } else if (pid == -1) {
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

// Function to construct the full path and check if the file exists
int constructAndCheckPath(char *out_ptr, char *path[], char *full_path) {
  for (int idx = 0; path[idx] != NULL; ++idx) {
      snprintf(full_path, PATH_MAX, "%s/%s", path[idx], out_ptr);

      if (access(full_path, X_OK) == 0) {
          return 1;  // File found and executable
      }
  }
  return 0;  // File not found in any path
}


// Function to split the command into arguments
void splitCommand(char *command, char *path[]) {
  int pids[50] = {0};
  char full_path[1024];  // Buffer for full path
  int command_exists_flag = 0;

  int i = 0;
  char *in_ptr = command;
  char *out_ptr;
  int pid_count = 0;
  const char *del1 = "&";
  const char *del = " ";


  char *commands[50] = {NULL};

  // If there is '&', then send command off and grab the new command
  // Will need to fork, then if it's the child you would break, if its the parent then we reset the args array and begin again

  // Check for ampersand first to split it up by multiple commands

  while ((out_ptr = strsep(&in_ptr, del1)) != NULL) {
      // Trim leading and trailing whitespace
      // Trim leading spaces
      while (isspace((unsigned char)*out_ptr)) out_ptr++;
      // Trim trailing spaces
      char *end = out_ptr + strlen(out_ptr) - 1;
      while (end > out_ptr && isspace((unsigned char)*end)) {
          *end = '\0';
          end--;
      }

      // Skip empty commands
      if (strlen(out_ptr) == 0) {
          continue;
      }
      commands[i] = out_ptr;
      i++;
  }


  for (i = 0; commands[i] != NULL; i++) {
      char *args[50] = {NULL};
      in_ptr = commands[i];
      int idx = 0;
      while ((out_ptr = strsep(&in_ptr, del)) != NULL) {
          args[idx] = out_ptr;
          idx++;
      }
      args[idx] = NULL;  // Null-terminate the args array
      // Now we need to complete this command
      // Check Path
      command_exists_flag = constructAndCheckPath(args[0], path, full_path);
      // Handle Command
      handleCommand(args, path, command_exists_flag, full_path);
      pid_count++;
  }

  // Wait for all child processes to finish
  for (int i = 0; i < pid_count; i++) {
      wait(NULL);
  }
}



int main() {

  char *command = NULL;
  size_t command_size = 0;


  //  Create a list to store the paths. Initially, the path should just be /bin/
  char *path[50] = {NULL};
  path[0] = "/bin";



  while (1) {
      printf("wish> ");
      size_t len = getline(&command, &command_size, stdin);

      // Not sure if I need this one
      if (len > 0 && command[len - 1] == '\n') {
          command[len - 1] = '\0';
      }
      //printf("You entered: %s\n", command);


      //Only the first command you check so yes we need to split it but we only check the first one.
      //Then could have redirections, multiple commands as well so need to look out for the & character and the > character
      splitCommand(command, path);
  }

  free(command);


  return 0;
}

