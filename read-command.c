// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <ctype.h>
#include "alloc.h"

/* FIXME: You may need to add #include directives, macro definitions,
	 static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
	 complete the incomplete type declaration in command.h.  */

typedef struct command_stream 
{
	// Array of pointers pointing to c-strings
	short size;
	short cmd_count;
	short line_count;
	char** command_list;
} command_stream;

// Trim whitespace off commands
char* trim_whitespace(char* str)
{
	char *end;
	while(isspace(*str)) str++;
	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;
	*(end+1) = 0;
	return str;
}

// Add command to command stream
int add_command(char*** list, int list_size, char* cmd, int cmd_size)
{
	if (cmd == NULL || *list == NULL) 
	{
		return 0;
	}

	int i = 0;

	char *l_cmd = (char*) checked_malloc(cmd_size+1);
	for (i = 0; i < cmd_size; i++)
	{
		l_cmd[i] = cmd[i];
	}
	// Append null byte
	l_cmd[cmd_size] = '\0';
	// Trim whitespace
	if(cmd[0] != '\n')
	{
		l_cmd = trim_whitespace(l_cmd);
	}
	else
	{
		if (list_size > 0 && cmd_size > 0)
		{
			if ((*list)[list_size-1][0] == '\n') 
			{
				return 0;
			}
		}
	}
	// Add command to list
	*list = (char**) checked_realloc(*list, (list_size+1)*sizeof(char**));
	(*list)[list_size] = strdup(l_cmd);

	return 1;
}

int is_valid_op(char op, char last_op) 
{
	return (op == '\n' || op == ' ' || op == ';' || 
			op == '#'  || op == '(' || op == ')' ||
			op == '&'  || op == '|' || op == '>' ||
			op == '<'  || last_op == '&' || last_op == '|' ||
			op == '\n');
}

	command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		void *get_next_byte_argument)
{

	/* FIXME: Replace this with your implementation.  You may need to
		 add auxiliary functions and otherwise modify the source code.
		 You can also use external functions defined in the GNU C Library.  */

	// initialize memory allocation
	// count of characters
	int pt_count = 0;
	int cmd_count = 0;
	char input_byte;
	char prev_byte;
	char *tmp = NULL;
	char *tmp_ch = NULL;
	command_stream_t result_stream = (struct command_stream*) checked_malloc(sizeof(struct command_stream*));
	result_stream->size = 0;
	result_stream->cmd_count = 0;
	result_stream->line_count = 1;

	// Initalize memory allocation for stream
	result_stream->command_list = (char**) checked_malloc(sizeof(char**));
	result_stream->command_list[0] = NULL;

	// initialize stream
	input_byte = (char)get_next_byte(get_next_byte_argument);

	// Reallocate memory for stream characters
	// Record stream
	while (input_byte != EOF)
	{    
		if (is_valid_op(input_byte, prev_byte))
		{
			// Add command
			if (tmp_ch != NULL) 
			{
				if (add_command(&(result_stream->command_list), pt_count, tmp_ch, cmd_count) != 0)
				{
					pt_count++;
				}
				else
				{
					prev_byte = input_byte;
					input_byte = get_next_byte(get_next_byte_argument);
					continue;
				}
				free(tmp_ch);
				tmp_ch = NULL;
			}
			// Operator Cases
			if (input_byte == '\n')
			{
        // Get rid of leading \n
        if(pt_count == 0)
        {
	        input_byte = (char)get_next_byte(get_next_byte_argument);
          continue;
        }
				  if(result_stream->command_list[pt_count-1][0] == '\n')
				  {
					  prev_byte = input_byte;
					  input_byte = get_next_byte(get_next_byte_argument);
					  continue;
				  }
			}
			if (input_byte == '#') 
			{
				// Append # comment to next command
				add_command(&(result_stream->command_list), pt_count++, "#", 1);
				//tmp_ch = (char*) checked_malloc(sizeof(char));
				//tmp_ch[0] = input_byte;
			}
			else if (input_byte == '|' && prev_byte == '|')
			{
				// Add operator 
				add_command(&(result_stream->command_list), pt_count++, "||", 2);
				// prevent from being read again
				input_byte = ' ';
				prev_byte = ' ';
			}
			else if (input_byte == '|' && prev_byte != '|')
			{
			}
			else if (input_byte != '|' && prev_byte == '|')
			{
				// Add operator 
				add_command(&(result_stream->command_list), pt_count++, "|", 1);
				prev_byte = ' ';
				cmd_count = 0;
				continue;
				// Add current
			}
			else if (input_byte == '&' && prev_byte != '&')
			{
			}
			else if (input_byte != '&' && prev_byte == '&')
			{
				// Add operator 
				add_command(&(result_stream->command_list), pt_count++, "&", 1);
				// Add current
				tmp_ch = (char*) checked_malloc(sizeof(char));
				tmp_ch[0] = input_byte;
			}
			else if (input_byte == '&' && prev_byte == '&')
			{
				// Add operator 
				add_command(&(result_stream->command_list), pt_count++, "&&", 2);
				// prevent from being read again
				input_byte = ' ';
				prev_byte = ' ';
			}
			else if (input_byte == ' ')
			{
			}
			else
			{
				// Add operator 
				tmp = (char*) checked_malloc(sizeof(char));
				tmp[0] = input_byte;
				add_command(&(result_stream->command_list), pt_count++, tmp, 1);
				free(tmp);
			} 
			// Reset for next add
			if( (prev_byte == '|' && !is_valid_op(input_byte, prev_byte)) || 
					(prev_byte == '&' && !is_valid_op(input_byte, prev_byte)))
			{
				cmd_count = 1;
			}
			else
			{
				cmd_count = 0;
			}
			prev_byte = input_byte;
			input_byte = get_next_byte(get_next_byte_argument);
			continue;
		}
		if (input_byte == '#') 
		{
			// Reset
			prev_byte = input_byte;
			input_byte = get_next_byte(get_next_byte_argument);
			continue;
		}
		// Allocate for command until delimiter found
		tmp_ch = (char*) checked_realloc(tmp_ch, (cmd_count+1)*sizeof(char));
		tmp_ch[cmd_count] = input_byte;
		cmd_count++;

		prev_byte = input_byte;
		input_byte = get_next_byte(get_next_byte_argument);
	}

	result_stream->size = pt_count;
	//Test print
  /*
	int i = 0;
	for (i=0; i < pt_count; i++)
	{
		printf("%d: %s \n", i, result_stream->command_list[i]);
	}
	printf("\npt_count: %d \n", pt_count);
	*/
	// Returns stream as character array
	return result_stream;
}



bool is_valid_char(char* c){
	//checks whether character is valid given requirements in spec
	bool valid_nonalnum = false;
	char valid_chars[21] = { '!', '%', '+', ',', '-', '.', '/', ':', '@', '^', '_', ';',
		'|', '&', '(', ')', '<', '>', '#', '\n', ' ' };
	size_t i = 0;
	size_t j = 0;
	for (j = 0; j < strlen(c); j++)
	{
		valid_nonalnum = false;
		for(i = 0; i < 21; i++)
		{
			if(c[j] == valid_chars[i]) 
			{
				valid_nonalnum = true;
			}
		}
		if(!(valid_nonalnum || isalnum(c[j])))
		{
			return false;
		}
	} 
	return true;
}

bool is_special_char(char *c)
{
	//determines whether a special character obeys grammar rules

	if(strcmp(c, "&&") == 0 ||
			strcmp(c, "||") == 0 ||
			strcmp(c, "|" ) == 0 ||
			strcmp(c, "&" ) == 0 ||
			strcmp(c, ";" ) == 0 ||
			strcmp(c, "<" ) == 0 ||
			strcmp(c, ">" ) == 0 ||
			strcmp(c, ")" ) == 0 ||
			strcmp(c, "(" ) == 0 )
		return true;

	return false;
}

// Make command object out of word
bool build_word_command(char** word, struct command **cmd_ptr)
{
	if (word == NULL)
		return false;
	(*cmd_ptr) = (struct command* )checked_malloc(sizeof(struct command));
	(*cmd_ptr)->type = SIMPLE_COMMAND;
	(*cmd_ptr)->status = 2;
	(*cmd_ptr)->input = NULL;
	(*cmd_ptr)->output = NULL;
	(*cmd_ptr)->u.word = word;
	return true;
}

// Make special command object given enum AND_COMMAND OR_COMMAND PIPE_COMMAND SEQUENCE_COMMAND
bool build_special_command(char* type, struct command **cmd_ptr)
{
	if (type == NULL)
		return false;
	if (strcmp(type,"&&") == 0)
	{
		(*cmd_ptr) = (struct command*)checked_malloc(sizeof(struct command));
		(*cmd_ptr)->type = AND_COMMAND;
	}
	else if (strcmp(type,"||") == 0)
	{
		(*cmd_ptr) = (struct command*)checked_malloc(sizeof(struct command));
		(*cmd_ptr)->type = OR_COMMAND;
	}
	else if (strcmp(type,"|") == 0)
	{
		(*cmd_ptr) = (struct command*)checked_malloc(sizeof(struct command));
		(*cmd_ptr)->type = PIPE_COMMAND;
	}
	else if (strcmp(type,";") == 0)
	{
		(*cmd_ptr) = (struct command*)checked_malloc(sizeof(struct command));
		(*cmd_ptr)->type = SEQUENCE_COMMAND;
	}
	else 
		return false;
	(*cmd_ptr)->status = 2;
	(*cmd_ptr)->input = NULL;
	(*cmd_ptr)->output = NULL;
	(*cmd_ptr)->u.command[0] = NULL; // Left Command
	(*cmd_ptr)->u.command[1] = NULL; // Right Command
	return true;


}

// Add cmd to sub command
bool build_sub_command(command_t* cmd, command_t *cmd_ptr)
{
	if (cmd == NULL)
		return false;
	(*cmd_ptr) = (struct command*)checked_malloc(sizeof(struct command));  
	(*cmd_ptr)->status = 2;
	(*cmd_ptr)->type = 5; //subshell code
	(*cmd_ptr)->input = NULL;
	(*cmd_ptr)->output = NULL;
	(*cmd_ptr)->u.subshell_command = *cmd;
	return true;
}

// Add command word to array list
bool add_cmd_to_list(char* cmd, char*** list, int list_size)
{
	*list = (char**)checked_realloc((*list), (list_size+100)*sizeof(char*));
	{
		if (cmd == NULL)
		{
			(*list)[list_size] = NULL;
		}
		else
		{
			(*list)[list_size] = strdup(cmd);
		}
		return true;
	}
	return false;
}


// Add list of array to direction of special command(&& || | ;)
bool add_cmd_to_special(command_t* cmd, command_t* special, char dir)
{
	if ((*cmd) == NULL || (*special) == NULL)
		return false;

	// Error Check
	if ((*cmd)->type == AND_COMMAND || (*cmd)->type == SEQUENCE_COMMAND || 
			(*cmd)->type == OR_COMMAND  || (*cmd)->type == PIPE_COMMAND)
	{
		if ((*cmd)->u.command[0] == NULL || (*cmd)->u.command[1] == NULL)
			return false;
	}

	// Add command to special command
	if (dir == 'l')
		(*special)->u.command[0] = *cmd;
	else if (dir == 'r')
	{
		if((*special)->u.command[0] == NULL)
			return false;

		(*special)->u.command[1] = *cmd;
	}
	else 
		return false;

	return true;
}

// Add cmd to input/output
bool add_word_to_IO(char* word, command_t* cmd, char io)
{
	if(cmd == NULL || word == NULL)
		return false;
	if (io == 'i')
	{
		(*cmd)->input = (char*)checked_malloc(strlen(word)*sizeof(char));
		(*cmd)->input = word;
	}
	else if (io == 'o')
	{
		(*cmd)->output = (char*)checked_malloc(strlen(word)*sizeof(char));
		(*cmd)->output = word;
	}
	else
		return false;
	return true;
}

void syn_error(command_stream_t s)
{
	fprintf(stderr, "syntax error on line: %d\n", s->line_count);
	exit(1);
}

bool break_tree(command_t c, command_t **output_cmd_array, int* array_size)
{
	//printf("Entering break_tree\n");
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
				(*output_cmd_array) = (command_t*)checked_realloc((*output_cmd_array), ((*array_size)+1)*sizeof(command_t)*100);
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
				(*output_cmd_array) = (command_t*)checked_realloc((*output_cmd_array), ((*array_size)+1)*sizeof(command_t)*100);
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

bool form_tree(command_t **c, command_t* output_cmd, int size)
{
	//given pointer to array of commands
	//printf("Entering form_tree with size %d\n", size);
	short i;
	int * broken_subshell_size=(int*)malloc(sizeof(int));
	*broken_subshell_size = 0;
	command_t * broken_subshell = (command_t*)malloc(sizeof(command_t));
	command_t subshell_output_cmd = (struct command*) checked_malloc(sizeof(struct command*));
	for(i = 0; i < size; i++)
	{
		if((*c)[i] == NULL) continue;
		//check for subcommands
		if((*c)[i]->type == SUBSHELL_COMMAND)
		{

			//break up the subshell tree
			if(!break_tree((*c)[i]->u.subshell_command, &broken_subshell, broken_subshell_size))
				printf("Error breaking subshell tree!\n");

			//pass in the broken tree to form_tree
			//command_t * temp_cmd_array = NULL;
			//temp_cmd_array = &(*c)[i];
			subshell_output_cmd = (struct command*) checked_realloc(subshell_output_cmd, (*broken_subshell_size)*sizeof(struct command*));
			if(!form_tree(&broken_subshell, &subshell_output_cmd, (*broken_subshell_size)))
				printf("Error forming subshell tree!\n");

			(*c)[i] = subshell_output_cmd;
		}
	}

	//PIPE_COMMANDS 
	for(i = 0; i < size; i++)
	{
		//printf("Pipe iteration number %d\n", i);
		if((*c)[i] == NULL)
		{
			//printf("continued\n");
			continue;
		}

		if((*c)[i]->type == PIPE_COMMAND)
		{
			if(i-1 >= 0)
			{
				int left_cmd_index = i-1;
				if((*c)[left_cmd_index] == NULL)
				{
					while(left_cmd_index >= 0 && (*c)[left_cmd_index] == NULL)
					{
						left_cmd_index--;
					}
					if((*c)[left_cmd_index] ==NULL) printf("Error attaching to left command of pipe");
				}

				//printf("Attaching %s command to pipe left\n", (*c)[left_cmd_index]->u.word[0]);
				(*c)[i]->u.command[0] = (*c)[left_cmd_index];
				(*c)[left_cmd_index] = NULL;

			}
			if(i+1 < size)
			{
				int right_cmd_index = i+1;
				if((*c)[right_cmd_index] ==NULL)
				{
					while(right_cmd_index < size && (*c)[right_cmd_index] ==NULL)
					{
						right_cmd_index++;
					}
				}

				//printf("Attaching %s command to pipe right\n", (*c)[right_cmd_index]->u.word[0]);
				(*c)[i]->u.command[1] = (*c)[right_cmd_index];
				(*c)[right_cmd_index] =NULL;
			}
		}
	}

	for(i = 0; i < size; i++)
	{
		//printf("ORANDSEQ Loop iteration %d\n", i);
		//OR COMMAND
		if((*c)[i] == NULL) continue;
		if((*c)[i]->type == OR_COMMAND || (*c)[i]->type == AND_COMMAND || (*c)[i]->type == SEQUENCE_COMMAND)
		{
			if(i-1 >= 0)
			{
				int left_cmd_index = i-1;
				//left is being accessed it has somethign to do with the while loop and the iterations of i--
				if((*c)[left_cmd_index] == NULL)
				{
					while(left_cmd_index >= 0 && (*c)[left_cmd_index] == NULL)
					{
						left_cmd_index--;
					}
					if((*c)[left_cmd_index] ==NULL)
					{
						printf("Error attaching to left command of or command\n");
					}
				}

				if((*c)[left_cmd_index]->type == SIMPLE_COMMAND)
				{
					//printf("Adding simple command %s to left of %d cmd\n", (*c)[left_cmd_index]->u.word[0], (*c)[i]->type);

				}
				else
				{
					//printf("Adding command %d to leeft of %d cmd\n",(*c)[left_cmd_index]->type, (*c)[i]->type);
				}
				(*c)[i]->u.command[0] = (*c)[left_cmd_index];
				(*c)[left_cmd_index] = NULL;

			}
			if(i+1 < size)
			{
				int right_cmd_index = i+1;
				if((*c)[right_cmd_index] ==NULL)
				{
					while(right_cmd_index < size && (*c)[right_cmd_index] ==NULL )
					{
						right_cmd_index++;
					}
				}

				if((*c)[right_cmd_index]->type == SIMPLE_COMMAND)
				{
					//printf("Adding simple %s to right of %d cmd\n", (*c)[right_cmd_index]->u.word[0], (*c)[i]->type);

				}
				else
				{
					//printf("Adding cmd %d to right of %d cmd\n", (*c)[right_cmd_index]->type, (*c)[i]->type);

				}

				(*c)[i]->u.command[1] = (*c)[right_cmd_index];
				(*c)[right_cmd_index] =NULL;
			}
		}
	}
	short cmd_count = 0;
	short cmd_index = 0;
	for(i = 0; i < size; i++)
	{
		if((*c)[i] != NULL)
		{
			cmd_count++;
			cmd_index=i;
		}
	}
	//printf("Cmd count: %d\n", cmd_count);
	//printf("Cmd index: %d\n", cmd_index);
	if(cmd_count != 1)
	{
		printf("Command count was not 1");
		return false;
	}

	//printf("I got to the end of form_tree\n");

	*output_cmd = (*c)[cmd_index];
	//printf("Output_cmd type: %d\n", (*output_cmd)->type);
	return true;
}

command_t
read_command_stream (command_stream_t s)
{
	/* 
		 take a command stream, return a command_t object to be used
		 by main.c
		 Read a command from STREAM; return it, or NULL on EOF.  If there is`
		 an error, report the error and exit instead of returning.  
Given: array of pointers to c strings (command_stream)
Output: singular constructed command objects such that
when read_command_stream is called one command
object is returned, and the next time it is 
called the second command object is returned, etc. command
object is returned, and the next time 
	 */

	int index = s->cmd_count;
	int list_size = s->size;

	struct command* cmd_ptr = NULL;
	struct command* special_ptr = NULL;
	struct command* outer_special_ptr = NULL;

	char ** word_list = NULL;
	int word_list_size = 0;
	//char ** comment_word_list = NULL;
	//int comment_word_list_size = 0;
	char dir = 'l';
	char outside_dir = 'l';

	bool found_start_subshell = false;
	bool only_comment_left = false;

	if (index > list_size-1)
		return NULL;

	// Skip repeating \n's
	if (strcmp(s->command_list[index], "\n") == 0 && cmd_ptr == NULL) {
		while ( strcmp(s->command_list[index], "\n") == 0 )
		{
			index++;
			s->line_count++;
			if (index > list_size-1)
				return NULL;
		}
	}
	//short loop_iteration = 0;
	while ( strcmp(s->command_list[index], "\n") != 0)
	{
		//		printf("Iteration: %d\n", loop_iteration++);
		// Syntax Check
		if (!is_valid_char(s->command_list[index]))
		{
			syn_error(s);
		}
		//if encounter a comment
		if(strcmp(s->command_list[index], "#") == 0)
		{
			//ignore commands being passed in until newline
			while(strcmp(s->command_list[index], "\n") != 0)
			{
				index++;
			}
			//if word list has things, continue to so that it will terminate
			//upon next iteration and construct the normal cmd
			if(word_list_size != 0) continue;
			//else keep going to next index as if never encountered a comment
			else index= index+1;
			//if the next thing is a \n, just break
			if(index+1 >= list_size)
			{
				only_comment_left = true;
				break;
			}
		}

		// For ooerators
		if(is_special_char(s->command_list[index]))
		{
			if(strcmp(s->command_list[index], "(") == 0)
			{
				if(found_start_subshell) syn_error(s);
				found_start_subshell = true;
				if(word_list_size !=0 && word_list != NULL && special_ptr==NULL)
					syn_error(s);

				outer_special_ptr = special_ptr;
				special_ptr = NULL;
				outside_dir = dir;
				dir = 'l';


			}
			else if(strcmp(s->command_list[index], ")")==0)
			{
				if(found_start_subshell)
				{
					found_start_subshell = false;
				}
				else
				{
					syn_error(s);
				}

				if(!build_word_command(word_list, &cmd_ptr))
				{
					syn_error(s);
				}
				word_list = NULL;
				word_list_size = 0;

				//if there is a special command

				if(special_ptr != NULL)
				{
					if(!add_cmd_to_special(&cmd_ptr, &special_ptr, 'r'))
						syn_error(s);
					cmd_ptr = special_ptr;
					special_ptr = NULL;

				}

				if(!build_sub_command(&cmd_ptr, &special_ptr))
					syn_error(s);
				cmd_ptr = special_ptr;
				special_ptr = outer_special_ptr;
				dir = outside_dir;

				//		printf("I came here!\n");
			}

			//check first for redirection special chars
			else if(strcmp(s->command_list[index], "<") == 0)
			{
				if (word_list != NULL)
				{
					if (!build_word_command(word_list, &cmd_ptr))
						syn_error(s);
					word_list = NULL;
					word_list_size = 0;
				}
				//if the next command = special char output an error
				if(index+1 < list_size)
				{
					if(is_special_char(s->command_list[index+1]) ||
							strcmp(s->command_list[index+1], "\n") == 0)
					{
						syn_error(s);
					}

					//else output an error
					if(!add_word_to_IO(s->command_list[index+1], &cmd_ptr, 'i'))
					{
						syn_error(s);
					}
					if( strcmp(s->command_list[index+2], "\n") == 0)
					{
						index++;
						break;
					}
				}

				//increment index an additional time because
				//command for index+1 is already constructed
				index++;
			}
			else if(strcmp(s->command_list[index], ">") ==0)
			{
				if (word_list != NULL)
				{
					if (!build_word_command(word_list, &cmd_ptr))
						syn_error(s);
					word_list = NULL;
					word_list_size = 0;
				}
				if(index+1 < list_size)//check condition later
				{
					if(is_special_char(s->command_list[index+1]) ||
							strcmp(s->command_list[index+1], "\n") == 0)
						syn_error(s);

					if(!add_word_to_IO(s->command_list[index+1], &cmd_ptr, 'o'))
						syn_error(s);
				}
				if( strcmp(s->command_list[index+2], "\n") == 0)
				{
					index++;
					break;
				}
				index++;
			}
			//else condition: not "<" or ">" so implement l and r ptrs
			// Add to left only in first iteration
			else if(dir == 'l')
			{
				if (word_list != NULL)
				{
					if (!build_word_command(word_list, &cmd_ptr))
						syn_error(s);
					word_list = NULL;
					word_list_size = 0;
				}

				if (!build_special_command(s->command_list[index], &special_ptr))
					syn_error(s);
				if (!add_cmd_to_special(&cmd_ptr,&special_ptr, 'l'))
					syn_error(s);
				cmd_ptr = NULL;
				dir = 'r';
			}
			// Add to further special commands
			else
			{
				if (word_list != NULL)
				{
					if (!build_word_command(word_list, &cmd_ptr))
						syn_error(s);
					word_list = NULL;
					word_list_size = 0;
				}
				if (!add_cmd_to_special(&cmd_ptr,&special_ptr,'r'))
					syn_error(s);
				cmd_ptr = special_ptr;
				special_ptr = NULL;
				if (!build_special_command(s->command_list[index], &special_ptr))
					syn_error(s);
				if (!add_cmd_to_special(&cmd_ptr,&special_ptr,'l'))
					syn_error(s);
				cmd_ptr = NULL;
			}



			//			while(index+2 < list_size && strcmp(s->command_list[index+1], "\n") == 0 && special_ptr != NULL)
			//		{
			//		index++;
			//	}
			/*

				 if(index+2 < list_size && strcmp(s->command_list[index+1], "\n") == 0 && special_ptr != NULL)
				 {
				 index++;

				 }

			 */

			if (index+2 < list_size)
			{
				if(strcmp(s->command_list[index+1], "\n") == 0 && special_ptr != NULL)
				{
					index++;
				}
			}
		}
		// For Single Words
		else
		{
			// Append to word_list before making commands
			if(!add_cmd_to_list(s->command_list[index], &word_list, word_list_size))
				syn_error(s);
			//    printf("Added command %s to position %d\n", s->command_list[index], word_list_size);
			word_list_size++;
		}
			index++;
	}
	s->cmd_count = index + 1;

	// Fix case for single sided sequence commands
	if (special_ptr != NULL)
	{
		if (special_ptr->type == SEQUENCE_COMMAND && special_ptr->u.command[1] == NULL)
		{
			add_cmd_to_list(" ", &word_list, word_list_size);
		} 
	}

	//  Build remaining list
	if (word_list != NULL)
	{
		if (!build_word_command(word_list, &cmd_ptr))
			syn_error(s);
		// int i;
		//	for(i = 0; i< word_list_size; i++)
		//	{
		//			printf("adding to cmd word %s\n", word_list[i]);

		//	}

		word_list_size = 0;
		word_list = NULL;
	}

	if(found_start_subshell)
		syn_error(s);

	// If remaining word_list commands and incomplete tree
	if (special_ptr != NULL)
	{
		if(!add_cmd_to_special(&cmd_ptr, &special_ptr, 'r'))
			syn_error(s);
		cmd_ptr = special_ptr;
		special_ptr = NULL;
	}

	//printf("I made it this far!");
	// Increment line count
	s->line_count++;
	if(only_comment_left==true)
	{
		return cmd_ptr;
	}
	command_t *cmd_array = (command_t*)checked_malloc(sizeof(command_t));
	int *array_size = (int*)checked_malloc(sizeof(int));

	*array_size = 0;
	break_tree(cmd_ptr, &cmd_array, array_size);
	command_t cmd = (struct command*) checked_malloc((*array_size)*sizeof(struct command *));
	form_tree(&cmd_array, &cmd, *array_size);
	
	return cmd;
}
