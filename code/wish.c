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

char *trimWhitespace(char *str) {
    if (str == NULL) return NULL;

    // 1. Advance past leading spaces
    while (isspace((unsigned char)*str)) {
        str++;
    }

    // If string is all spaces, return the empty string
    if (*str == '\0') {
        return str;
    }

    // 2. Trim trailing spaces
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return str;
}

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
          // Child process: Execute the command
          execv(full_path, args);
          // If execv fails, exit the child process
          //perror("execv failed");
          //exit(1);
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

void handleRedirectionCommand(int command_exists_flag, char *full_path, char *args[], char *output) {
  //printf("%d\n", 1);
  //printf("Command FLAG: %d\n", command_exists_flag);
  if (command_exists_flag == 1) {
      pid_t pid = fork();
      if (pid == 0) {
          // Open the file and redirect stdout to it
          freopen(output, "w", stdout);
          //printf("FULL PATH: %s\n", full_path);
          // Child process: Execute the command
          execv(full_path, args);
          freopen("/dev/tty", "w", stdout);
          // If execv fails, exit the child process
          //perror("execv failed");
          //exit(1);
      } else if (pid == -1) {
          // Fork failed
          perror("fork failed");
      }
  } else {
      // Command not found: Print an error
      handleError();
  }
}

// Create function for dealing with redirection
void handleRedirection(char *redirection, char *path[], char *full_path) {
    // Need to break apart the command into 2 sections
    // 1. Command part
    // 2. Redirection into output file

    char *out_ptr;
    char *in_ptr = redirection;
    const char *del = ">";
    const char *del1 = " ";

    char *command;
    char *output_file;

    // Check for ampersand first to split it up by multiple commands
    int i = 0;
    while ((out_ptr = strsep(&in_ptr, del)) != NULL) {
        out_ptr = trimWhitespace(out_ptr);

        // Skip empty commands
        if (strlen(out_ptr) == 0) {
            continue;
        }
        if (i == 0) {
            command = out_ptr;
        } else if (i == 1) {
            output_file = out_ptr;
        } else {
            // There is multiple files after redirection so return the error code
            handleError();
        }
        i++;
    }

    in_ptr = command;

    char *args[50] = {NULL};
    int command_exists_flag = 0;

    int idx = 0;
    while ((out_ptr = strsep(&in_ptr, del1)) != NULL) {
        args[idx] = out_ptr;
        idx++;
    }
    args[idx] = NULL;  // Null-terminate the args array
    // Now we need to complete this command
    // Check Path
    command_exists_flag = constructAndCheckPath(args[0], path, full_path);
    // Handle Command
    handleRedirectionCommand(command_exists_flag, full_path, args, output_file);

    // Now to create an output file with the contents of the command in it

    // Now handle the command and it will go into that file

    printf("HANDLING REDIRECTION!\n");
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
      out_ptr = trimWhitespace(out_ptr);

      // Skip empty commands
      if (strlen(out_ptr) == 0) {
          continue;
      }
      commands[i] = out_ptr;
      i++;
  }


  for (i = 0; commands[i] != NULL; i++) {
      in_ptr = commands[i];
      int redirection_flag = 0;
      int error_flag = 0;
      // Check if there is a redirection in the command
      for (int idx = 0; in_ptr[idx] != NULL; idx++) {
          if (in_ptr[idx] == '>') {
              if (redirection_flag == 1) {
                  // Then we have an error so display error and go to next command
                  handleError();
                  error_flag = 1;
                  break;
              }
              redirection_flag = 1;
          }
      }
      if (error_flag == 1) {
          continue;
      }
      else if (redirection_flag == 1) {
          handleRedirection(commands[i], path, full_path);
          pid_count++;
      } else {
          char *args[50] = {NULL};

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

