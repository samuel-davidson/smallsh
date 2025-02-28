/**************************************************************
Citations:

1. Command Line parser & struct taken from sample_parser.c
    (Lines 34-40, 88-112)
2. Forking and waiting for child process adapted from Module 6,
	Exploration 3 (Process API - Monitoring Child Processes)
	(Code example #3). (Lines 65-83)
3. Status display adapted from Module 6, Exploration 3 (Process API
	- Monitoring Child Processes) (Code example #4). (Lines 120-134)
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
#include <fcntl.h>


#define INPUT_LENGTH     2048
#define MAX_ARGS		 512

struct command_line *parse_input();
int handle_exit(pid_t groupID);
void handle_status(int childStatus);
void handle_cd(struct command_line *curr_command);
void handle_input_redirection(struct command_line *curr_command);
void handle_output_redirection(struct command_line *curr_command);

struct command_line {
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
};

int main() {
	struct command_line *curr_command;
	int childStatus;

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
			handle_exit(groupID);
		} else if (strcmp(curr_command->argv[0], "cd") == 0) {
			// cd command
			handle_cd(curr_command);
		} else if (strcmp(curr_command->argv[0], "status") == 0) { 
			// status command
			handle_status(childStatus);
		} else {
			//int childStatus;
			pid_t pid = fork();
			if (pid == -1) {
				perror("fork() failed!"); 
				fflush(stdout);
				exit(EXIT_FAILURE); 
				break;
			} else if (pid == 0) {
			// child process executes this
			// input redirection
				if (curr_command->input_file) {
					handle_input_redirection(curr_command);
				}
				// output redirection
				if (curr_command->output_file) {
					handle_output_redirection(curr_command);
				} else {
					execvp(curr_command->argv[0], curr_command->argv);
					printf("%s: no such command\n", curr_command->argv[0]);
					fflush(stdout);
					exit(1);
				}
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
	return curr_command;
}

int handle_exit(pid_t groupID) {
	// kills any other process via group ID
	killpg(groupID, SIGKILL);
	return EXIT_SUCCESS;
}

void handle_status(int childStatus) {
	if(WIFEXITED(childStatus)) {
		// normal termination
		printf("exit value %d\n", WEXITSTATUS(childStatus));
		fflush(stdout);
	} else if (WIFSIGNALED(childStatus)) {
		// abnormal termination
		printf("terminated by signal %d\n", WTERMSIG(childStatus));
		fflush(stdout);
	} else {
		// error
		printf("error determining exit value\n");
		fflush(stdout);
	}
}

void handle_cd(struct command_line *curr_command) {
	// check if cd is alone in input
	if (curr_command->argc == 1) {
		// go to home env
		char * home = getenv("HOME");
		chdir(home);
	} else {
		// change the directory
		if (chdir(curr_command->argv[1]) == -1) {
			printf("%s: no such file or directory\n", curr_command->argv[1]);
			fflush(stdout);
		}
	}
}

void handle_input_redirection(struct command_line *curr_command) {
	// handles input redirection
	//printf("I AM BEING REACHED: INPUT");
	int sourceFD = open(curr_command->input_file, O_RDONLY);
	if (sourceFD == -1) { 
		perror("source open()"); 
		exit(1); 
	  }
	int result = dup2(sourceFD, 0);
  	if (result == -1) { 
    	perror("source dup2()"); 
    	exit(2); 
  	}
  	close(sourceFD);
}

void handle_output_redirection(struct command_line *curr_command) {
	// handles output redirection
	//printf("I AM BEING REACHED: OUTPUT");
	int targetFD = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(targetFD == -1) { 
		perror("target open()"); 
		exit(1);
	}
	int result = dup2(targetFD, 1);
	if(result == -1) { 
		perror("target dup2()");
		exit(2);
	}
	close(targetFD);
}