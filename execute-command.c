// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* FIXME: You may need to add #include directives, macro definitions,
	 static function definitions, etc.  */

/* ERROR CDOES:
 * -1 = RUNNING
 *  0 = EXIT_SUCCESS
 *  1 = EXIT_FAILURE
 *  2 = YET_TO_BE_RUN
 */

void cmd_error(const char* cmd)
{
	if (cmd == NULL)
		exit(1);
	fprintf(stderr, "Command: %s cannot be found.\n", cmd);
	exit(1);
}

bool run_command(command_t c)
{
	if (c == NULL)
	{
		return false;
	}
	// Check IO
	// Set contents of file to be an argument
	if (c->input != NULL)
	{
		size_t i = 0;
		size_t in_size = strlen(c->input);
		// Find end of command list
		while (c->u.word[i] != NULL)
		{
			i++; 
		}
		c->u.word[i] = (char*)malloc(in_size *sizeof(char));
		c->u.word[i] = c->input;
		c->u.word[i+1] = (char*)malloc(sizeof(char));
		c->u.word[i+1] = NULL;
	} 

	// Print stdout to file
	if (c->output != NULL)
	{
		freopen(c->output, "w", stdout);
	}

	// Testing Code
	printf("\n\nCommand: %s \nWith Arguments: %s \n", c->u.word[0], c->u.word[1] );

	if (execvp(c->u.word[0], c->u.word) == -1)
	{
		c->status = 1;
		cmd_error(c->u.word[0]);
	}

	// Close output file if set
	if (c->output != NULL)
	{
		fclose(stdout);
	}
	return true;
}

void free_command(command_t c)
{
	switch(c->type)
	{
		case SIMPLE_COMMAND:
			{
				free(c);
				break;
			}
		case SUBSHELL_COMMAND:
			{
				free_command(c->u.subshell_command);
				break;
			}
		case AND_COMMAND:
		case OR_COMMAND:
		case PIPE_COMMAND:
		case SEQUENCE_COMMAND:
			{
				free_command(c->u.command[0]);
				printf("NO ERROR HERE\n");
				free_command(c->u.command[1]);
				break;
			}
	}
	return;
}

bool recurse_command(command_t c)
{
	int pipefd[2];
	int status = 0;
	pid_t in_pid, out_pid;

	// Error Check
	if (c == NULL)
	{
		return true;
	}

	// Recurse through children
	switch(c->type)
	{
		case AND_COMMAND:
			{
				return recurse_command(c->u.command[0]) && recurse_command(c->u.command[1]);
			}
		case OR_COMMAND:
			{
				return recurse_command(c->u.command[0]) || recurse_command(c->u.command[1]);
			}
		case PIPE_COMMAND:
			{
				status = 0;
				pipe(pipefd);
				// Generate output to pipe
				out_pid = fork();
				if (out_pid == 0)
				{
					dup2(pipefd[1], STDOUT_FILENO);
					close(pipefd[0]);
					recurse_command(c->u.command[0]);
				}
				else if (out_pid > 0)
					wait(&status);
				else if (out_pid < 0)
				{
					fprintf(stderr, "Fork Error");
					exit(1);
				}

				// Generate input to pipe
				in_pid = fork();
				if (in_pid == 0)
				{
					dup2(pipefd[0], STDIN_FILENO);
					close(pipefd[1]);
					recurse_command(c->u.command[1]);
				}
				else if (in_pid > 0)
					wait(&status);
				else if (in_pid < 0)
				{
					fprintf(stderr, "Fork Error");
					exit(1);
				}
				return true;

			}
		case SEQUENCE_COMMAND:
			{
				recurse_command(c->u.command[0]);
				recurse_command(c->u.command[1]);
				return true;
			}
		case SUBSHELL_COMMAND:
			{
				return recurse_command(c->u.subshell_command);
			}
		case SIMPLE_COMMAND:
			{
				status = 0;
				// Put command in separate process
				pid_t pid = fork();
				if (pid > 0)
					wait(&status);
				else if (pid == 0)
				{
					if (!isspace(c->u.word[0][0]))
					{
						run_command(c);
					}
				} 
				else if (pid < 0)
				{
					fprintf(stderr, "Fork Error");
					exit(1);
				}

				if (status > 1)
					return false;
			}
		default:
			{
				return true;
			}
	}
}

bool break_tree(command_t c, command_t **output_cmd_array, int* array_size)
{
	if ( c == NULL)
		return false;
	switch(c->type)
	{
		// In order traversal to break tree.
		// Makes leaf nodes NULL
		case AND_COMMAND:
		case OR_COMMAND:
		case PIPE_COMMAND:
		case SEQUENCE_COMMAND:
			{
				break_tree(c->u.command[0], output_cmd_array, array_size);
				(*output_cmd_array) = (command_t*)realloc((*output_cmd_array), ((*array_size)+1)*sizeof(command_t)*100);
				(*output_cmd_array)[(*array_size)] = c;
				(*array_size)++;
				break_tree(c->u.command[1], output_cmd_array, array_size);
				break;
			}
			// Adds command to command list
			// Subshell does not get broken up
		case SUBSHELL_COMMAND:
		case SIMPLE_COMMAND:
			{
				(*output_cmd_array) = (command_t*)realloc((*output_cmd_array), ((*array_size)+1)*sizeof(command_t)*100);
				(*output_cmd_array)[(*array_size)] = c;

				(*array_size)++;
				break;
			}
		default:
			{
				return false;
			}
	}
	return true;

}

bool form_tree(command_t *c, command_t output_cmd, int size)
{
	//given pointer to array of commands
	//
	output_cmd = output_cmd;
	int i;
	int * broken_subshell_size=NULL;
	for(i = 0; i < size; i++)
	{
		if(c[i] == NULL) continue;
		//check for subcommands
		if(c[i]->type == SUBSHELL_COMMAND)
		{
			command_t * broken_subshell = NULL;
			
			//break up the subshell tree
			if(!break_tree(c[i], &broken_subshell, broken_subshell_size))
				printf("Error breaking subshell tree!\n");

			command_t subshell_output_cmd = NULL;
			//pass in the broken tree to form_tree
			if(!form_tree(&c[i], subshell_output_cmd, (*broken_subshell_size)))
				printf("Error forming subshell tree!\n");


			c[i] = subshell_output_cmd;
		}
	}

	//PIPE_COMMANDS 
	for(i = 0; i < size; i++)
	{
		if(c[i] == NULL) continue;

		if(c[i]->type == PIPE_COMMAND)
		{
			if(i-1 >= 0)
			{
				if(c[i-1] == NULL)
				{
					while(c[i-1] == NULL)
						i--;
					if(c[i-1] ==NULL) printf("Error attaching to left command of pipe");
							}
							}

							c[i]->u.command[0] = c[i-1];
							c[i-1] = NULL;

							if(i+1 < size)
							{
							if(c[i+1] ==NULL)
							{
							while(c[i+1] ==NULL)
							i++;
							}
							}
							c[i]->u.command[1] = c[i+1];
							c[i+1] =NULL;
							}

							}

	for(i = 0; i < size; i++)
	{
		//OR COMMAND
		if(c[i] == NULL) continue;
		if(c[i]->type == OR_COMMAND || c[i]->type == AND_COMMAND || c[i]->type == SEQUENCE_COMMAND)
		{
			if(i-1 >= 0)
			{
				if(c[i-1] == NULL)
				{
					while(c[i-1] == NULL)
						i--;
					if(c[i-1] ==NULL) printf("Error attaching to left command of or command\n");
				}
			}
			c[i]->u.command[0] = c[i-1];
			c[i-1] = NULL;

			if(i+1 < size)
			{
				if(c[i+1] ==NULL)
				{
					while(c[i+1] ==NULL)
						i++;
				}
			}
			c[i]->u.command[1] = c[i+1];
			c[i+1] =NULL;
		}
	}
/*
	for(i = 0; i < size; i++)
	{
		//AND COMMAND
		if(c[i] == NULL) continue;
		if(c[i]->type == AND_COMMAND)
		{
			if(i-1 >= 0)
			{
				if(c[i-1] == NULL)
				{
					while(c[i-1] == NULL)
						i--;
					if(c[i-1] ==NULL) printf("Error attaching to left command of AND command\n");
				}
			}
			c[i]->command[0] = c[i-1];
			c[i-1] = NULL;

			if(i+1 < size)
			{
				if(c[i+1] ==NULL)
				{
					while(c[i+1] ==NULL)
						i++;
				}
			}
			c[i]->command[1] = c[i+1];
			c[i+1] =NULL;
		}
	}
	for(i = 0; i < size; i++)
	{
		//SEQUENCE COMMAND
		if(c[i] == NULL) continue;
		if(c[i]->type == SEQUENCE_COMMAND)
		{
			if(i-1 >= 0)
			{
				if(c[i-1] == NULL)
				{
					while(c[i-1] == NULL)
						i--;
					if(c[i-1] ==NULL) printf("Error attaching to left command of or command\n");
				}
			}
			c[i]->command[0] = c[i-1];
			c[i-1] = NULL;

			if(i+1 < size)
			{
				if(c[i+1] ==NULL)
				{
					while(c[i+1] ==NULL)
						i++;
				}
			}
			c[i]->command[1] = c[i+1];
			c[i+1] =NULL;
		}
	}

*/
	int cmd_count =0;
	int cmd_index = 0;
	for(i = 0; i < size; i++)
	{
		if(c[i] != NULL)
		{
			cmd_count++;
			cmd_index=i;
		}
	}
	if(cmd_count != 1)
		return false;


	output_cmd = c[cmd_index];
	return true;
}

	int
command_status (command_t c)
{
	return c->status;
}

void test_output_cmd(command_t *cmd_array, int array_size)
{ 
	// Test break tree
	int i = 0;
	for (i = 0; i < array_size ;i++)
	{
		switch(cmd_array[i]->type)
		{
			case AND_COMMAND:
				printf("Type: AND_COMMAND\n"); break;
			case SEQUENCE_COMMAND:
				printf("Type: SEQENCE_COMMAND\n"); break;
			case OR_COMMAND:
				printf("Type: PIPE_COMMAND\n"); break;
			case PIPE_COMMAND:
				printf("Type: PIPE_COMMAND\n"); break;
			case SIMPLE_COMMAND:
				printf("Type: SIMPLE_COMMAND\n Word: %s\n", cmd_array[i]->u.word[0]); break;
			case SUBSHELL_COMMAND:
				printf("Type: SUBSHELL_COMMAND\n"); break;
		}
	}
}

	void
execute_command (command_t c, bool time_travel)
{
	/* FIXME: Replace this with your implementation.  You may need to
		 add auxiliary functions and otherwise modify the source code.
		 You can also use external functions defined in the GNU C Library.  */
	if (time_travel)
	{
		// Implement Me
		error (1, 0, "time travel execution not yet implemented");
		return;
	}
	else
	{
		command_t *cmd_array = (command_t*)malloc(sizeof(command_t));
		int *array_size = (int*)malloc(sizeof(int));
		*array_size = 0;
		break_tree(c, &cmd_array, array_size);
		test_output_cmd(cmd_array, *array_size);

		//recurse_command(c);
	}


	return;
}
