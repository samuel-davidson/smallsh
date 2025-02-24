/**************************************************************
Citations:

1. Command Line parser & struct taken from sample_parser.c
    (Lines 30-36, 91-120)
2. Forking and waiting for child process adapted from Module 6,
	Exploration 3 (Process API - Monitoring Child Processes).
	(Lines 68-86)
3.
4.
5.
6.
***************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define INPUT_LENGTH     2048
#define MAX_ARGS		 512

struct command_line *parse_input();

struct command_line {
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
};

int main() {
	struct command_line *curr_command;

	while(true) {
		curr_command = parse_input();

		// HANDLE BUILT IN COMMANDS
		if (curr_command->argv[0] == NULL) { continue; }
			// handle empty lines 
		else if (strncmp(curr_command->argv[0], "#", 1) == 0) { continue; }
			// handle comments
		else if (strcmp(curr_command->argv[0], "exit") == 0) {
			// exit command
			// kills any other process via group ID
			pid_t groupID = getpgrp();
			killpg(groupID, SIGKILL);
			return EXIT_SUCCESS;
		} else if (strcmp(curr_command->argv[0], "cd") == 0) {
			// cd command
			// check if cd is alone in input
			if (curr_command->argc == 1) {
				// go to home env
				char * home = getenv("HOME");
				chdir(home);
			} else {
				// change the directory
				if (chdir(curr_command->argv[1]) == -1) {
					printf("%s: no such file or directory\n", curr_command->argv[1]);
				} else { continue; }
			}
		} else {
			int childStatus;
			pid_t pid = fork();
			if (pid == -1) {
				perror("fork() failed!"); 
				exit(EXIT_FAILURE); 
				break;
			} else if (pid == 0) {
			// child process executes this
				execvp(curr_command->argv[0], curr_command->argv);
				printf("%s: no such command\n", curr_command->argv[0]);
				fflush(stdout);
				exit(1);
			} else if (pid > 0) {
			// parent process executes this
				waitpid(pid, &childStatus, 0);
			}
		}
	}
	return EXIT_SUCCESS;
}

struct command_line *parse_input() {
	char input[INPUT_LENGTH];
	struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

	// Get input
	printf(": ");
	fflush(stdout);
	fgets(input, INPUT_LENGTH, stdin);

	// debugging print statement
	// printf("INPUT = %s", input);

	// Tokenize the input
	char *token = strtok(input, " \n");
	while(token){
		if(!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			curr_command->is_bg = true;
		} else{
			curr_command->argv[curr_command->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}
	// debugging print statement
	// printf("CURRENT COMMAND = %s %s\n", curr_command->argv[0], curr_command->argv[1]);
	return curr_command;
}