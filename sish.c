#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 1024
#define MAX_ARGUMENTS 64
#define MAX_HISTORY_SIZE 100

char history_list[MAX_HISTORY_SIZE][MAX_INPUT_LENGTH];
int history_count = 0;
char command[MAX_INPUT_LENGTH];
char *arguments[MAX_ARGUMENTS];

void parse_command(char *input_command, char *command, char *arguments[]);
void execute_builtin_command(char *command, char *arguments[]);
void execute_command(char *command, char *arguments[]);
void read_input(char *input_string);
void update_history(char *input_command);

// Function to parse input command into command and arguments
void parse_command(char *input_command, char *command, char *arguments[]) {
   char *token;
   int arg_index = 0;

   // Tokenize input_command by space
   token = strtok(input_command, " ");

   // Copy the first token as command
   if (token != NULL) {
       strcpy(command, token);
   } else {
       strcpy(command, "");
       return;
   }

   // Iterate over the tokens
   while (token != NULL) {
       // Store the token as an argument
       arguments[arg_index++] = token;

       token = strtok(NULL, " "); // Get the next token
   }

   arguments[arg_index] = NULL; // Set the last argument as NULL for execvp
}


// Function to execute built-in commands
void execute_builtin_command(char *command, char *arguments[]) {
  if (strcmp(command, "exit") == 0) {
      exit(0); // Exit the shell
  } else if (strcmp(command, "cd") == 0) {
      if (arguments[1] == NULL) {
          printf("cd: missing directory\n"); // Print error message if directory is missing
      } else {
          if (chdir(arguments[1]) == -1) {
              printf("cd: unable to change directory\n"); // Print error message if chdir fails
          }
      }
  } else if (strcmp(command, "history") == 0) {
      if (arguments[1] == NULL) {
          // Display last 100 commands with offsets
          int start_index = (history_count > 100) ? history_count - 100 : 0;
          for (int i = start_index; i < history_count; i++) {
              printf("%d %s\n", i, history_list[i % MAX_HISTORY_SIZE]);
          }
      } else if (strcmp(arguments[1], "-c") == 0) {
          // Clear command history
          history_count = 0;
      } else {
          // Check if the argument is a valid integer
          char *endptr;
          long index = strtol(arguments[1], &endptr, 10);
          if (*endptr != '\0') {
              printf("history: invalid index\n"); // Print error message if index is not a number
          } else if (index < 0 || index >= history_count) {
              printf("history: index out of range\n"); // Print error message if index is out of range
          } else {
               // Check if the argument is a valid integer
               char *endptr;
               long index = strtol(arguments[1], &endptr, 10);
               if (*endptr != '\0') {
                   printf("history: invalid index\n"); // Print error message if index is not a number
                   exit;
               } else if (index < 0 || index >= history_count) {
                   printf("history: index out of range\n"); // Print error message if index is out of range
               } else {
                   // Execute command from history
                   char input_command[MAX_INPUT_LENGTH];
                   strcpy(input_command, history_list[index]);
                   char command[MAX_INPUT_LENGTH];
                   char *new_arguments[MAX_ARGUMENTS];
                   parse_command(input_command, command, new_arguments);
                   if (strcmp(command, "") != 0) {
                       if (strcmp(command, "exit") == 0) {
                           exit(0);
                       }
                       if (strcmp(command, "cd") == 0 || strcmp(command, "history") == 0) {
                           execute_builtin_command(command, new_arguments);
                       } else {
                           execute_command(command, new_arguments);
                       }
                   }
               }
           }
       }
   }
}

// Function to execute external commands
void execute_command(char *command, char *arguments[]) {
   pid_t pid = fork();
   if (pid == -1) {
       perror("fork");
       exit(EXIT_FAILURE);
   } else if (pid == 0) {  // Child process
       if (execvp(command, arguments) == -1) {
           perror("execvp");
           exit(EXIT_FAILURE);
       }
   } else {  // Parent process
       wait(NULL);  // Wait for child process to complete
   }
}

// Function to read input from stdin
void read_input(char *input_string) {
  printf("sish> "); // Display shell prompt
  fgets(input_string, MAX_INPUT_LENGTH, stdin); // Read input from stdin
  input_string[strcspn(input_string, "\n")] = '\0'; // Remove trailing newline character
}

// Function to update command history
void update_history(char *input_command) {
  if (history_count < MAX_HISTORY_SIZE) {
      strcpy(history_list[history_count++], input_command); // Add command to history list
  } else {
      // Shift history list to make room for new command
      for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
          strcpy(history_list[i], history_list[i + 1]);
      }
      strcpy(history_list[MAX_HISTORY_SIZE - 1], input_command); // Add command to history list
  }
}

int main() {
  char input_command[MAX_INPUT_LENGTH];

  while (1) {
      read_input(input_command); // Read input from user
      update_history(input_command); // Update command history
      parse_command(input_command, command, arguments); // Parse input command
      if (strcmp(command, "") != 0) { // Check if command is not empty
          if (strcmp(command, "exit") == 0) {
              exit(0); // Exit the shell if the command is "exit"
          }
          if (strcmp(command, "cd") == 0) {
              execute_builtin_command(command, arguments); // Execute built-in command "cd"
          } else if (strcmp(command, "history") == 0) {
              execute_builtin_command(command, arguments); // Execute built-in command "history"
          } else {
              execute_command(command, arguments); // Execute external command
          }
      }
  }

  return 0;
}
