# smallsh

Small Shell


### Currently Implemented and Working:

- Provide a prompt for running commands
- Handle blank lines and comments, which are lines beginning with the # character
- Execute other commands by creating new processes using a function from the exec() family of functions

### Implemented but Needs Work:

- Execute 3 commands exit (DONE), cd (DONE), and status via code built into the shell

### Needs to be Implemented:

- Support input and output redirection
- Support running commands in foreground and background processes
- Implement custom handlers for 2 signals, SIGINT and SIGTSTP
