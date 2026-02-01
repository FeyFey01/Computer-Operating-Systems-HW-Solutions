#include <shell.h>      // this was already given, it also imports all the allowed libraries except wait.h
#include <sys/wait.h>   // include this to use waitpid 

// I tried to add comments to each line
void executor(List *command_list)
{
    int num_commands = 0;    // we're gonna count how many commands we have in the list
    for (Node *node = command_list->head; node; node = node->next)   // loop through each command in the list
        num_commands++;     // increment the count for each command node

    int pipefd[2];         // for connecting commands
    int prev_fd = -1;      // end of the previous pipe to link them up
    pid_t *pids = malloc(num_commands * sizeof(pid_t));  // manually allocate memory to store all the pids for each executable

    int i = 0;             // counter for iterating thru commands
    for (Node *node = command_list->head; node; node = node->next, i++) 
    {
        command *cmd = node->content;  // get the command

        // if exit command handle it seperately bc this wasnt working otherwise
		if (strcmp(cmd->args[0], "exit") == 0)
			exit(0);    // exit with 0 

        // if cd command just like exit
		if (strcmp(cmd->args[0], "cd") == 0)  
		{
			if (cmd->args[1] == NULL)
			{
				fprintf(stderr, "cd: missing argument\n");    // print error message if argument is missing
				cmd->exit_status = 1;  // set exit status 1 as error
			}
			else if (chdir(cmd->args[1]) != 0)  // try changing directory
			{
				perror("cd");    // print the error message if fails
				cmd->exit_status = 1;   // exit status 1
			}
			else
			{
				cmd->exit_status = 0;	// set exit status as 0 (success)
			}
			continue;   // skip to the next command if cd is executed
		}

		// if there's more than one command given, create a pipe
		if (node->next != NULL)    // check if the next node exists
		{
			if (pipe(pipefd) == -1)    // if pipe creation fails
			{
				perror("pipe failed");    // print the error
				exit(EXIT_FAILURE);       // exit the program with failure
			}
		}

		// fork the process for each command
		pid_t pid = fork();  // fork a new process
		if (pid == -1)   // if fork fails
		{
			perror("fork failed");    // print error message
			exit(EXIT_FAILURE);       // exit with failure
		}

        if (pid == 0) // doesnt fail
        {
            // for commands like < input.txt
            if (cmd->fd_in != 0)  // if theres an input
            {
                dup2(cmd->fd_in, STDIN_FILENO);  // input to stdin
                close(cmd->fd_in);    // close the original fd since it's now duplicated
            }
            else if (prev_fd != -1)  // otherwise, use the previous pipe's output
            {
                dup2(prev_fd, STDIN_FILENO);    // redirect the input from previous pipe's read end
                close(prev_fd);  // close the prev_fd after redirection
            }

            // for commands like > output.txt
            if (cmd->fd_out != 1)  // if theress an output 
            {
                dup2(cmd->fd_out, STDOUT_FILENO);   // output to stdout
                close(cmd->fd_out);   // close the original fd after duplicating it
            }
            else if (node->next != NULL)    // if not the last command in the pipeline, send output to next pipe
            {
                close(pipefd[0]);    // close the read end of the pipe
                dup2(pipefd[1], STDOUT_FILENO);    // send output to pipe's write end
                close(pipefd[1]);    // close write end after redirection
            }

            // clean up any unused file descriptors in child
            if (prev_fd != -1)
                close(prev_fd);   // close prev_fd if it's open
            if (node->next != NULL)
                close(pipefd[0]);    // close the pipe's read end in child process

            // execute the command using execvp
            if (execvp(cmd->args[0], cmd->args) == -1)
            {
                perror("execvp failed");  // if execvp fails, print error
                exit(EXIT_FAILURE);   // exit with failure code
            }
        }

        pids[i] = pid;    // store the child process

        // close the file descriptors that are no longer needed in the parent process
        if (prev_fd != -1)
            close(prev_fd);   // close the previous fd in parent
        if (node->next != NULL)
        {
            close(pipefd[1]);       // close the pipe's write end
            prev_fd = pipefd[0];    // store the read end of the pipe
        }
    }

    // Wait for all of them and then get their exit status
    i = 0;  // reset counter
    for (Node *node = command_list->head; node; node = node->next, i++)   // loop through all commands again
    {
        int status;   // store exit status
        waitpid(pids[i], &status, 0);   // wait for it to finish

        command *cmd = node->content;    // get the command
        if (WIFEXITED(status))    // if the child process ended normally with 0 as status
            cmd->exit_status = WEXITSTATUS(status);   // store the exit status of the command
        else
            cmd->exit_status = -1;   // set exit status as -1 if it ended eith error
    }

    free(pids);   // free the memory allocated
}
