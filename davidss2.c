/**************************************************************
Citations:

1. Command Line parser & struct taken from sample_parser.c
    (lines 23-29, 42-66)
2. 
3.
4.
5.
6.
***************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

	while(true)
	{
		curr_command = parse_input();

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