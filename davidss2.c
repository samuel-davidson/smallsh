/**************************************************************
Citations:

1. Command Line parser & struct taken from sample_parser.c
    (Lines 38-44, 98-122)
2. Forking and waiting for child process adapted from Module 6,
	Exploration 3 (Process API - Monitoring Child Processes)
	(Code example #3). (Lines 69-93)
3. Status display adapted from Module 6, Exploration 3 (Process API
	- Monitoring Child Processes) (Code example #4). (Lines 130-144)
4. Input and output redirection adapted from Module 7, Exploration 4 
	(Processes and I/O) (Code example #3). (Lines 161-175 and 177-191)
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
int handle_input_redirection(struct command_line *curr_command);
int handle_output_redirection(struct command_line *curr_command);

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
	int background_procs[64] = {};

	while(true) {
		// check BG processes for completion before input
		for (int i = 0; i < 64; i++){
			int exitStatus;
			if (background_procs[i] > 0){
				if (waitpid(background_procs[i], &exitStatus, WNOHANG) > 0){
					printf("background pid %d is done: ", background_procs[i]);
					fflush(stdout);
					handle_status(exitStatus);
					background_procs[i] = 0;
				}
			}
		}
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
			pid_t pid = fork();
			if (pid == -1) {
				perror("fork() failed!"); 
				fflush(stdout);
				exit(EXIT_FAILURE); 
				break;
			} else if (pid == 0) {
			// child process executes this
				if (curr_command->input_file) {
					// input redirection
					handle_input_redirection(curr_command);
				} else if (curr_command->is_bg) {
					// background with no input redir
					handle_input_redirection(curr_command);
				} if (curr_command->output_file) {
					// output redirection
					handle_output_redirection(curr_command);
				} else if (curr_command->is_bg) {
					// background with no output redir
					handle_output_redirection(curr_command);
				}
				execvp(curr_command->argv[0], curr_command->argv);
				printf("%s: no such command\n", curr_command->argv[0]);
				fflush(stdout);
				exit(1);
			} if (!curr_command->is_bg){
				// FG
				waitpid(pid, &childStatus, 0);
			} else {
				// BG
				printf("background pid is %d\n", pid);
				fflush(stdout);
				// add to array to check if completed before next prompt
				for (int i = 0; i < 64; i++){
					if (background_procs[i] == 0){
						background_procs[i] = pid; break;
					}
				}
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

int handle_input_redirection(struct command_line *curr_command) {
	// handles input redirection
	int sourceFD;
	if (curr_command->is_bg) {
		sourceFD = open("/dev/null", O_RDONLY);
	} else {
		sourceFD = open(curr_command->input_file, O_RDONLY);
	}
	if (sourceFD == -1) { 
		perror("source open()"); 
		exit(1); 
	} 
	int result = dup2(sourceFD, 0);
  	if (result == -1) { 
    	perror("source dup2()"); 
    	exit(2); 
  	}
	return 0;
}

int handle_output_redirection(struct command_line *curr_command) {
	// handles output redirection
	int targetFD;
	if (curr_command->is_bg) {
		targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	} else {
		targetFD = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	}
	if(targetFD == -1) { 
		perror("target open()"); 
		exit(1);
	}
	int result = dup2(targetFD, 1);
	if(result == -1) { 
		perror("target dup2()");
		exit(2);
	}
	return 0;
}